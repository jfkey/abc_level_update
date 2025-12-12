/**CFile****************************************************************

  FileName    [abcRewrite.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Network and node package.]

  Synopsis    [Technology-independent resynthesis of the AIG based on DAG aware rewriting.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 20, 2005.]

  Revision    [$Id: abcRewrite.c,v 1.00 2005/06/20 00:00:00 alanmi Exp $]

***********************************************************************/

#include "base/abc/abc.h"
#include "opt/rwr/rwr.h"
#include "bool/dec/dec.h"


ABC_NAMESPACE_IMPL_START

 
extern abctime global_time; 
extern abctime global_resynthesis_time; 
extern abctime global_cut_time; 
extern abctime global_aig_update_time; 
extern abctime global_aig_converter_time;  
extern long int global_level_updates;
extern long int global_reverse_updates;
extern long int global_node_rewritten; 
extern long int global_reorder_nodes;
extern long int max_aff_size; 
/*
    The ideas realized in this package are inspired by the paper:
    Per Bjesse, Arne Boralv, "DAG-aware circuit compression for 
    formal verification", Proc. ICCAD 2004, pp. 42-49.
*/

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

static Cut_Man_t * Abc_NtkStartCutManForRewrite( Abc_Ntk_t * pNtk );
static void        Abc_NodePrintCuts( Abc_Obj_t * pNode );
static void        Abc_ManShowCutCone( Abc_Obj_t * pNode, Vec_Ptr_t * vLeaves );

extern void  Abc_PlaceBegin( Abc_Ntk_t * pNtk );
extern void  Abc_PlaceEnd( Abc_Ntk_t * pNtk );
extern void  Abc_PlaceUpdate( Vec_Ptr_t * vAddedCells, Vec_Ptr_t * vUpdatedNets );

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Performs incremental rewriting of the AIG.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Abc_NtkRewrite( Abc_Ntk_t * pNtk, int fUpdateLevel, int fUseZeros, int fVerbose, int fVeryVerbose, int fPlaceEnable )
{
    extern int           Dec_GraphUpdateNetwork( Abc_Obj_t * pRoot, Dec_Graph_t * pGraph, int fUpdateLevel, int nGain );
    ProgressBar * pProgress;
    Cut_Man_t * pManCut;
    Rwr_Man_t * pManRwr;
    Abc_Obj_t * pNode;
//    Vec_Ptr_t * vAddedCells = NULL, * vUpdatedNets = NULL;
    Dec_Graph_t * pGraph;
    int i, nNodes, nGain, fCompl, RetValue = 1;
    abctime clk, clkStart = Abc_Clock();

    assert( Abc_NtkIsStrash(pNtk) );
    // cleanup the AIG
     
    // Abc_AigCleanup((Abc_Aig_t *)pNtk->pManFunc);
    Abc_AigCleanupInc((Abc_Aig_t *)pNtk->pManFunc);
    
 
    global_time = 0; 
    global_resynthesis_time = 0; 
    global_cut_time = 0; 
    global_aig_update_time = 0; 
    global_aig_converter_time = 0;  
    global_level_updates = 0;
    global_reverse_updates = 0;
    global_node_rewritten = 0;
    global_reorder_nodes = 0;
    max_aff_size = 0; 
/*
    {
        Vec_Vec_t * vParts;
        vParts = Abc_NtkPartitionSmart( pNtk, 50, 1 );
        Vec_VecFree( vParts );
    }
*/

    // start placement package
//    if ( fPlaceEnable )
//    {
//        Abc_PlaceBegin( pNtk );
//        vAddedCells = Abc_AigUpdateStart( pNtk->pManFunc, &vUpdatedNets );
//    }

    // start the rewriting manager
    pManRwr = Rwr_ManStart( 0 );
    if ( pManRwr == NULL )
        return 0;
    // compute the reverse levels if level update is requested
    if ( fUpdateLevel )
        Abc_NtkStartReverseLevels( pNtk, 0 );
    // start the cut manager
clk = Abc_Clock();
    pManCut = Abc_NtkStartCutManForRewrite( pNtk );
Rwr_ManAddTimeCuts( pManRwr, Abc_Clock() - clk );
    pNtk->pManCut = pManCut;

    if ( fVeryVerbose )
        Rwr_ScoresClean( pManRwr );

    // resynthesize each node once
    pManRwr->nNodesBeg = Abc_NtkNodeNum(pNtk);
    nNodes = Abc_NtkObjNumMax(pNtk);
    pProgress = Extra_ProgressBarStart( stdout, nNodes );
    
    // initialize the linked list of nodes
    // Abc_Aig_t * pMan = (Abc_Aig_t *)pNtk->pManFunc;
    List_Ptr_Node_t * oLNode; 
    List_Ptr_t * oList = Abc_AigGetOList((Abc_Aig_t *)pNtk->pManFunc);
    oList->nSize = 0;
    
    // with representation of order by List 
    Abc_NtkForEachNode( pNtk, pNode, i ){
        oLNode = List_PtrPushBack( oList, pNode );
        assert(oLNode != NULL);
        pNode ->oLNode = oLNode;  
    }
    i = -1; 
    int max_node_id = Abc_NtkObjNumMax(pNtk); 
    oList->pCurItera = List_PtrFirstNode(oList);
    for (; oList->pCurItera != NULL; oList->pCurItera = oList->pCurItera ->pNext) { 
        pNode = (Abc_Obj_t *) oList->pCurItera->pData; 
        i ++;  
        // different from the condition in Abc_NtkForEachNode
        // this condition is used to avoid the nodes that may be deleted not in the nework 
        // we delete the node from AIG but do not remove the order from the linked list 
        if (pNode == NULL || !Abc_ObjIsNode(pNode)) continue;
        Extra_ProgressBarUpdate( pProgress, i, NULL );
  
        if (pNode->fHandled){
            printf("Abc_NtkRewrite: node %d has been handled.\n", pNode->Id); 
            continue;
        }
      
        // iterative for original nodes and newly created nodes. 
        // // stop if all nodes have been tried once
        if ( i >= nNodes )
            break;

        // skip persistant nodes
        if ( Abc_NodeIsPersistant(pNode) ){
            if (fUpdateLevel) {
                pNode->fHandled = 1;
                Abc_AigUpdateLevel_Lazy( pNode);
            } 
            continue;
        }
  
        // skip the nodes with many fanouts
        if ( Abc_ObjFanoutNum(pNode) > 1000 ){
            if (fUpdateLevel) {
                pNode->fHandled = 1;
                Abc_AigUpdateLevel_Lazy( pNode);
            } 
            continue;
        }
        // if (pNode->Id == 300281) 
            // printf("currrent node id %d \n", pNode->Id); 

        // assert the node's fanin has been handled
        if (fUpdateLevel){
            Abc_Obj_t * pFanin0 = Abc_ObjFanin0(pNode); 
            Abc_Obj_t * pFanin1 = Abc_ObjFanin1(pNode);         
            if (pFanin0 != NULL) 
            if (!Abc_ObjIsCi(pFanin0) && pFanin0->oLNode != NULL)
                assert(pFanin0->fHandled);
            if (pFanin1 != NULL)
            if (!Abc_ObjIsCi(pFanin1) && pFanin1->oLNode != NULL)
                assert(pFanin1->fHandled);
             
            Abc_AigUpdateLevel_Lazy(pNode); 
        }
         
        // avoid persistently rewriting newly created nodes with zero gain 
        if (pNode->Id > max_node_id) {
            if (fUpdateLevel)  pNode->fHandled = 1;  
            i --;
            continue;
        }
       

        // for each cut, try to resynthesize it
        nGain = Rwr_NodeRewrite( pManRwr, pManCut, pNode, fUpdateLevel, fUseZeros, fPlaceEnable );
       

        if ( !(nGain > 0 || (nGain == 0 && fUseZeros)) ){
            if (fUpdateLevel)  pNode->fHandled = 1;  
            continue;
        } 

        // if we end up here, a rewriting step is accepted
          
        // get hold of the new subgraph to be added to the AIG
        pGraph = (Dec_Graph_t *)Rwr_ManReadDecs(pManRwr);
        fCompl = Rwr_ManReadCompl(pManRwr);

        // reset the array of the changed nodes
        if ( fPlaceEnable )
            Abc_AigUpdateReset( (Abc_Aig_t *)pNtk->pManFunc );
  
        // complement the FF if needed
        if ( fCompl ) Dec_GraphComplement( pGraph );
clk = Abc_Clock(); 
    if ( !Dec_GraphUpdateNetwork( pNode, pGraph, fUpdateLevel, nGain ) )
        {
            if (fUpdateLevel)  pNode->fHandled = 1;
            RetValue = -1;
            break;
        } 
Rwr_ManAddTimeUpdate( pManRwr, Abc_Clock() - clk );
    if (fCompl) Dec_GraphComplement(pGraph);
        // use the array of changed nodes to update placement
//        if ( fPlaceEnable )
//            Abc_PlaceUpdate( vAddedCells, vUpdatedNets ); 
        global_node_rewritten += 1; 
        if ( fUpdateLevel ){
            pNode->fHandled = 1;
            if (!Abc_AigReplaceUpdateAff( (Abc_Aig_t *)pNtk->pManFunc)){
                RetValue = -1; 
                break; 
            } 
        }  
    } 
    Extra_ProgressBarStop( pProgress );
     

Rwr_ManAddTimeTotal( pManRwr, Abc_Clock() - clkStart );

    ABC_PRT("###global_time ",                   pManRwr->timeTotal);   
    ABC_PRT("###global_cut ",                    pManRwr->timeCut); 
    ABC_PRT("###global_resynthesis_time",        pManRwr->timeRes); 
    ABC_PRT("###global_aig_update_time",         global_aig_update_time); 
    ABC_PRT("###global_aig_converter_time ",     pManRwr->timeUpdate); 
      
    printf("###global_level_updates \t %ld\n",    global_level_updates);   
    printf("###global_reverse_updates \t %ld\n",  global_reverse_updates);   
    printf("###global_node_rewritten \t %ld\n",   global_node_rewritten);       
    printf("###global_reorder_nodes \t %ld\n",    global_reorder_nodes);
    printf("###max_aff_size \t %ld\n",            max_aff_size);

    // print stats
    pManRwr->nNodesEnd = Abc_NtkNodeNum(pNtk);
    if ( fVerbose )
        Rwr_ManPrintStats( pManRwr );
//        Rwr_ManPrintStatsFile( pManRwr );
    if ( fVeryVerbose )
        Rwr_ScoresReport( pManRwr );
    // delete the managers
    Rwr_ManStop( pManRwr );
    Cut_ManStop( pManCut );
    pNtk->pManCut = NULL;

    // clear the mark of fHandled nodes
    List_Ptr_Iterator_t * oLIter = (List_Ptr_Iterator_t*)ABC_ALLOC(List_Ptr_Iterator_t, 1);
    List_PtrForEach(List_Ptr_t*, oList, pNode, oLIter ){ 
        if (pNode == NULL || !Abc_ObjIsNode(pNode)) continue;
        if (pNode->Id > max_node_id) {
            pNode->fHandled = 0;
            continue;
        }
        // printf("after delete node: %d \n", pNode->Id);
        if ( pNode->Level != 1 + (unsigned)Abc_MaxInt( Abc_ObjFanin0(pNode)->Level, Abc_ObjFanin1(pNode)->Level ) )
            printf( "Abc_AigCheck immediately after rewrite: Node \"%d\" (%d) handled has level that does not agree with the fanin levels.\n", pNode->fHandled, Abc_ObjId(pNode) );
        pNode->fHandled = 0;  
    }
    List_PtrClear(oList);
    ABC_FREE(oLIter);
 

    // start placement package
//    if ( fPlaceEnable )
//    {
//        Abc_PlaceEnd( pNtk );
//        Abc_AigUpdateStop( pNtk->pManFunc );
//    }

    // put the nodes into the DFS order and reassign their IDs
    {
//        abctime clk = Abc_Clock();
    Abc_NtkReassignIds( pNtk );
//        ABC_PRT( "time", Abc_Clock() - clk );
    }
//    Abc_AigCheckFaninOrder( pNtk->pManFunc );
    // fix the levels
    if ( RetValue >= 0 )
    {
        if ( fUpdateLevel ){
            Abc_NtkLevel( pNtk );
            Abc_NtkStopReverseLevels( pNtk );
        } 
        else
            Abc_NtkLevel( pNtk );
        // check
        if ( !Abc_NtkCheck( pNtk ) )
        {
            printf( "Abc_NtkRewrite: The network check has failed.\n" );
            return 0;
        }
    }
    return RetValue;
}
 


/**Function*************************************************************

  Synopsis    [Starts the cut manager for rewriting.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Cut_Man_t * Abc_NtkStartCutManForRewrite( Abc_Ntk_t * pNtk )
{
    static Cut_Params_t Params, * pParams = &Params;
    Cut_Man_t * pManCut;
    Abc_Obj_t * pObj;
    int i;
    // start the cut manager
    memset( pParams, 0, sizeof(Cut_Params_t) );
    pParams->nVarsMax  = 4;     // the max cut size ("k" of the k-feasible cuts)
    pParams->nKeepMax  = 250;   // the max number of cuts kept at a node
    pParams->fTruth    = 1;     // compute truth tables
    pParams->fFilter   = 1;     // filter dominated cuts
    pParams->fSeq      = 0;     // compute sequential cuts
    pParams->fDrop     = 0;     // drop cuts on the fly
    pParams->fVerbose  = 0;     // the verbosiness flag
    pParams->nIdsMax   = Abc_NtkObjNumMax( pNtk );
    pManCut = Cut_ManStart( pParams );
    if ( pParams->fDrop )
        Cut_ManSetFanoutCounts( pManCut, Abc_NtkFanoutCounts(pNtk) );
    // set cuts for PIs
    Abc_NtkForEachCi( pNtk, pObj, i )
        if ( Abc_ObjFanoutNum(pObj) > 0 )
            Cut_NodeSetTriv( pManCut, pObj->Id );
    return pManCut;
}

/**Function*************************************************************

  Synopsis    [Prints the cuts at the nodes.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Abc_NodePrintCuts( Abc_Obj_t * pNode )
{
    Vec_Ptr_t * vCuts;
    Cut_Cut_t * pCut;
    int k;

    printf( "\nNode %s\n", Abc_ObjName(pNode) );
    vCuts = (Vec_Ptr_t *)pNode->pCopy;
    Vec_PtrForEachEntry( Cut_Cut_t *, vCuts, pCut, k )
    {
        Extra_PrintBinary( stdout, (unsigned *)&pCut->uSign, 16 ); 
        printf( "   " );
        Cut_CutPrint( pCut, 0 );   
        printf( "\n" );
    }
}


/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Abc_ManRewritePrintDivs( Vec_Ptr_t * vDivs, int nLeaves )
{
    Abc_Obj_t * pFanin, * pNode, * pRoot;
    int i, k;
    pRoot = (Abc_Obj_t *)Vec_PtrEntryLast(vDivs);
    // print the nodes
    Vec_PtrForEachEntry( Abc_Obj_t *, vDivs, pNode, i )
    {
        if ( i < nLeaves )
        {
            printf( "%6d : %c\n", pNode->Id, 'a'+i );
            continue;
        }
        printf( "%6d : %2d = ", pNode->Id, i );
        // find the first fanin
        Vec_PtrForEachEntry( Abc_Obj_t *, vDivs, pFanin, k )
            if ( Abc_ObjFanin0(pNode) == pFanin )
                break;
        if ( k < nLeaves )
            printf( "%c", 'a' + k );
        else
            printf( "%d", k );
        printf( "%s ", Abc_ObjFaninC0(pNode)? "\'" : "" );
        // find the second fanin
        Vec_PtrForEachEntry( Abc_Obj_t *, vDivs, pFanin, k )
            if ( Abc_ObjFanin1(pNode) == pFanin )
                break;
        if ( k < nLeaves )
            printf( "%c", 'a' + k );
        else
            printf( "%d", k );
        printf( "%s ", Abc_ObjFaninC1(pNode)? "\'" : "" );
        if ( pNode == pRoot )
            printf( " root" );
        printf( "\n" );
    }
    printf( "\n" );
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Abc_ManShowCutCone_rec( Abc_Obj_t * pNode, Vec_Ptr_t * vDivs )
{
    if ( Abc_NodeIsTravIdCurrent(pNode) )
        return;
    Abc_NodeSetTravIdCurrent(pNode);
    Abc_ManShowCutCone_rec( Abc_ObjFanin0(pNode), vDivs );
    Abc_ManShowCutCone_rec( Abc_ObjFanin1(pNode), vDivs );
    Vec_PtrPush( vDivs, pNode );
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Abc_ManShowCutCone( Abc_Obj_t * pNode, Vec_Ptr_t * vLeaves )
{
    Abc_Ntk_t * pNtk = pNode->pNtk;
    Abc_Obj_t * pObj;
    Vec_Ptr_t * vDivs;
    int i;
    vDivs = Vec_PtrAlloc( 100 );
    Abc_NtkIncrementTravId( pNtk );
    Vec_PtrForEachEntry( Abc_Obj_t *, vLeaves, pObj, i )
    {
        Abc_NodeSetTravIdCurrent( Abc_ObjRegular(pObj) );
        Vec_PtrPush( vDivs, Abc_ObjRegular(pObj) );
    }
    Abc_ManShowCutCone_rec( pNode, vDivs );
    Abc_ManRewritePrintDivs( vDivs, Vec_PtrSize(vLeaves) );
    Vec_PtrFree( vDivs );
}


/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Abc_RwrExpWithCut_rec( Abc_Obj_t * pNode, Vec_Ptr_t * vLeaves, int fUseA )
{
    if ( Vec_PtrFind(vLeaves, pNode) >= 0 || Vec_PtrFind(vLeaves, Abc_ObjNot(pNode)) >= 0 )
    {
        if ( fUseA )
            Abc_ObjRegular(pNode)->fMarkA = 1;
        else
            Abc_ObjRegular(pNode)->fMarkB = 1;
        return;
    }
    assert( Abc_ObjIsNode(pNode) );
    Abc_RwrExpWithCut_rec( Abc_ObjFanin0(pNode), vLeaves, fUseA );
    Abc_RwrExpWithCut_rec( Abc_ObjFanin1(pNode), vLeaves, fUseA );
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Abc_RwrExpWithCut( Abc_Obj_t * pNode, Vec_Ptr_t * vLeaves )
{
    Abc_Obj_t * pObj;
    int i, CountA, CountB;
    Abc_RwrExpWithCut_rec( Abc_ObjFanin0(pNode), vLeaves, 1 );
    Abc_RwrExpWithCut_rec( Abc_ObjFanin1(pNode), vLeaves, 0 );
    CountA = CountB = 0;
    Vec_PtrForEachEntry( Abc_Obj_t *, vLeaves, pObj, i )
    {
        CountA += Abc_ObjRegular(pObj)->fMarkA;
        CountB += Abc_ObjRegular(pObj)->fMarkB;
        Abc_ObjRegular(pObj)->fMarkA = 0;
        Abc_ObjRegular(pObj)->fMarkB = 0;
    }
    printf( "(%d,%d:%d) ", CountA, CountB, CountA+CountB-Vec_PtrSize(vLeaves) );
}


////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

