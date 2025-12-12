# import sys
# import re
# import pandas as pd

# # Check if the file path is provided as a command-line argument
# if len(sys.argv) < 2:
#     print("Usage: python extRes.py <file_path>")
#     sys.exit(1)

# # The first command-line argument after the script name is the file path
# file_path = sys.argv[1]
 
 
# parseRes = dict()
# type = 'arith'
 

# def parseOriginalRes(text_data): 
#     split_original_res_key = '###'
#     # extract the content before "statistics" onwards
#     start_idx = text_data.find(split_original_res_key)
#     #extract the desgin name from the content, eg. tv80.aig
#     if start_idx == -1:
#         print("Split key {} not found".format(split_original_res_key))
#         return 
#     truncated_text = text_data[:start_idx] 
     
#     design_name = re.search(r'/.*?/([^/]+)\.aig', truncated_text) 
#     pattern = r'i/o\s*=\s*(\d+)/\s*(\d+).*?and\s*=\s*(\d+).*?lev\s*=\s*(\d+)'
#     match = re.search(pattern, truncated_text, re.DOTALL)

#     if match:
#         i, o, and_num, lev = match.groups()
#         return {
#                 'design': design_name.group(1),
#                 '#input': int(i),
#                 '#output': int(o),
#                 '#and': int(and_num),
#                 '#level': int(lev)  
#             }
#     else:
#         print("No match found for original res")

# '''
# global_time 
# global_cut 
# global_resynthesis_time 
# global_aig_update_time 
# global_aig_converter_time 
# global_level_updates 
# global_reverse_updates 
# global_node_rewritten 
# and 
# lev
# '''
# def parseRewriteRes(text_data):
#     split_original_res_key = '###'
#     start_idx = text_data.find(split_original_res_key)
#     if start_idx == -1:
#         print("Split key {} not found".format(split_original_res_key))
#         return 
#     truncated_text = text_data[start_idx:]  
#     res_dict = {}
     
#     # use one dict to store the result
#     key = 'global_time'
#     pattern = rf"{key}\s*=\s*([\d.]+|\d+)"
#     match = re.search(pattern, truncated_text, re.DOTALL) 
#     if match:
#         res_dict['all_time'] = match.group(1)
        
    
#     key = 'global_cut'
#     pattern = rf"{key}\s*=\s*([\d.]+|\d+)"
#     match = re.search(pattern, truncated_text, re.DOTALL) 
#     if match:
#         res_dict['cut_time'] = match.group(1)
    
#     key = 'global_resynthesis_time'
#     pattern = rf"{key}\s*=\s*([\d.]+|\d+)"
#     match = re.search(pattern, truncated_text, re.DOTALL) 
#     if match:
#         res_dict['resyn_time'] = match.group(1)
    
#     key = 'global_aig_update_time'
#     pattern = rf"{key}\s*=\s*([\d.]+|\d+)"
#     match = re.search(pattern, truncated_text, re.DOTALL) 
#     if match:
#         res_dict['lev_upd_time'] = match.group(1)
    
#     key = 'global_aig_converter_time'
#     pattern = rf"{key}\s*=\s*([\d.]+|\d+)"
#     match = re.search(pattern, truncated_text, re.DOTALL) 
#     if match:
#         res_dict['aig_conv_time'] = match.group(1)
    
#     key = 'global_level_updates'
#     pattern = rf"{key}\s*(\d+\.?\d*|\d*\.?\d+)"
#     match = re.search(pattern, truncated_text, re.DOTALL) 
#     if match:
#         res_dict['lev_upd_num'] = int(match.group(1))
    
#     key = 'global_reverse_updates'
#     pattern = rf"{key}\s*(\d+\.?\d*|\d*\.?\d+)"
#     match = re.search(pattern, truncated_text, re.DOTALL) 
#     if match:
#         res_dict['rev_upd_num'] = int(match.group(1))
    
#     key = 'global_node_rewritten'
#     pattern = rf"{key}\s*(\d+\.?\d*|\d*\.?\d+)"
#     match = re.search(pattern, truncated_text, re.DOTALL) 
#     if match:
#         res_dict['gain_nodes'] = int(match.group(1))

#     key = 'global_reorder_nodes'
#     pattern = rf"{key}\s*(\d+\.?\d*|\d*\.?\d+)"
#     match = re.search(pattern, truncated_text, re.DOTALL) 
#     if match:
#         res_dict['reorder_nodes'] = int(match.group(1))
    
#     key = 'and'
#     pattern = rf"{key}\s*=\s*(\d+)"
#     match = re.search(pattern, truncated_text, re.DOTALL) 
#     if match:
#         res_dict['#and_rw'] = int(match.group(1))
    
#     key = 'lev'
#     pattern = rf"{key}\s*=\s*(\d+)"
#     match = re.search(pattern, truncated_text, re.DOTALL) 
#     if match:
#         res_dict['#lev_rw'] = match.group(1)

#     return res_dict


 
 
# with open(file_path, 'r') as file:
#     # Iterate over each line in the file
   
#     resRewrite = []
#     resRefactor = []
#     resResub = [] 
#     for line in file:  
#         original_res = parseOriginalRes(line)
#         if "rewrite" in line:

#             rewrite_res = parseRewriteRes(line) 
#             if original_res and rewrite_res:
#                 combined_res = {**original_res, **rewrite_res}  # 合并两个字典
#                 resRewrite.append(combined_res)
#             elif original_res:
#                 resRewrite.append(original_res)  # 如果只有原始数据
#             elif rewrite_res:
#                 resRewrite.append(rewrite_res)  # 如果只有重写数据

#         if "refactor" in line:
#             refactor_res = parseRewriteRes(line)
#             if original_res and refactor_res:
#                 combined_res = {**original_res, **refactor_res}
#                 resRefactor.append(combined_res)
#             elif original_res:
#                 resRefactor.append(original_res)
#             elif refactor_res:
#                 resRefactor.append(refactor_res)
        
#         if "resub" in line:
#             resub_res = parseRewriteRes(line)
#             if original_res and resub_res:
#                 combined_res = {**original_res, **resub_res}
#                 resResub.append(combined_res)
#             elif original_res:
#                 resResub.append(original_res)
#             elif resub_res:
#                 resResub.append(resub_res)
                
#     dfRewrite = pd.DataFrame(resRewrite)
#     dfRewrite = dfRewrite.sort_values(by='#and', ascending=True)
#     # print(dfRewrite)
    
#     # dfRefactor = pd.DataFrame(resRefactor)
#     # dfRefactor = dfRefactor.sort_values(by='#and', ascending=True)
#     # # print(dfRefactor)
    
#     # dfResub = pd.DataFrame(resResub)
#     # dfResub = dfResub.sort_values(by='#and', ascending=True)
#     # print(dfResub)
    
#     # write to current path/file_name.log
#     path = file_path.split('/')[-1]
#     dfRewrite.to_csv('./exp-analysis/'+path.split('.')[0]+'_rewrite.csv', index=False)
#     # dfRefactor.to_csv('./exp-analysis/'+path.split('.')[0]+'_refactor.csv', index=False)
#     # dfResub.to_csv('./exp-analysis/'+path.split('.')[0]+'_resub.csv', index=False)

import sys
import re
import pandas as pd

# Check if the file path is provided as a command-line argument
if len(sys.argv) < 2:
    print("Usage: python extRes.py <file_path>")
    sys.exit(1)

# The first command-line argument after the script name is the file path
file_path = sys.argv[1]

parseRes = dict()
type = 'arith'

def parseOriginalRes(text_data): 
    split_original_res_key = '###'
    # extract the content before "statistics" onwards
    start_idx = text_data.find(split_original_res_key)
    
    if start_idx == -1:
        # 如果找不到 split key，说明这一行可能格式不对，直接跳过
        # print("Split key {} not found".format(split_original_res_key))
        return None
        
    truncated_text = text_data[:start_idx] 
    
    # --- 修改点 1: 更稳健的正则表达式提取文件名 ---
    # 逻辑：寻找以 .aig 结尾，且前面不是斜杠或空格的字符串
    # 这里的 ([^/\\ ]+) 捕获组就是文件名（不含扩展名）
    design_name_match = re.search(r'([^/\\ ]+)\.aig', truncated_text)
    
    design_val = "unknown"
    if design_name_match:
        design_val = design_name_match.group(1)
    
    pattern = r'i/o\s*=\s*(\d+)/\s*(\d+).*?and\s*=\s*(\d+).*?lev\s*=\s*(\d+)'
    match = re.search(pattern, truncated_text, re.DOTALL)

    if match:
        i, o, and_num, lev = match.groups()
        return {
                'design': design_val,  # 这里的键名 design 要和后面 DataFrame 列名对应
                '#input': int(i),
                '#output': int(o),
                '#and': int(and_num),
                '#level': int(lev)  
            }
    else:
        # 调试用，如果提取数据失败打印提示
        # print("No match found for statistics in original res")
        return None

def parseRewriteRes(text_data):
    split_original_res_key = '###'
    start_idx = text_data.find(split_original_res_key)
    if start_idx == -1:
        return None
        
    truncated_text = text_data[start_idx:]  
    res_dict = {}
     
    # use one dict to store the result
    key = 'global_time'
    pattern = rf"{key}\s*=\s*([\d.]+|\d+)"
    match = re.search(pattern, truncated_text, re.DOTALL) 
    if match:
        res_dict['all_time'] = match.group(1)
        
    key = 'global_cut'
    pattern = rf"{key}\s*=\s*([\d.]+|\d+)"
    match = re.search(pattern, truncated_text, re.DOTALL) 
    if match:
        res_dict['cut_time'] = match.group(1)
    
    key = 'global_resynthesis_time'
    pattern = rf"{key}\s*=\s*([\d.]+|\d+)"
    match = re.search(pattern, truncated_text, re.DOTALL) 
    if match:
        res_dict['resyn_time'] = match.group(1)
    
    key = 'global_aig_update_time'
    pattern = rf"{key}\s*=\s*([\d.]+|\d+)"
    match = re.search(pattern, truncated_text, re.DOTALL) 
    if match:
        res_dict['lev_upd_time'] = match.group(1)
    
    key = 'global_aig_converter_time'
    pattern = rf"{key}\s*=\s*([\d.]+|\d+)"
    match = re.search(pattern, truncated_text, re.DOTALL) 
    if match:
        res_dict['aig_conv_time'] = match.group(1)
    
    key = 'global_level_updates'
    pattern = rf"{key}\s*(\d+\.?\d*|\d*\.?\d+)"
    match = re.search(pattern, truncated_text, re.DOTALL) 
    if match:
        res_dict['lev_upd_num'] = int(match.group(1))
    
    key = 'global_reverse_updates'
    pattern = rf"{key}\s*(\d+\.?\d*|\d*\.?\d+)"
    match = re.search(pattern, truncated_text, re.DOTALL) 
    if match:
        res_dict['rev_upd_num'] = int(match.group(1))
    
    key = 'global_node_rewritten'
    pattern = rf"{key}\s*(\d+\.?\d*|\d*\.?\d+)"
    match = re.search(pattern, truncated_text, re.DOTALL) 
    if match:
        res_dict['gain_nodes'] = int(match.group(1))

    key = 'global_reorder_nodes'
    pattern = rf"{key}\s*(\d+\.?\d*|\d*\.?\d+)"
    match = re.search(pattern, truncated_text, re.DOTALL) 
    if match:
        res_dict['reorder_nodes'] = int(match.group(1))
    
    key = 'and'
    pattern = rf"{key}\s*=\s*(\d+)"
    match = re.search(pattern, truncated_text, re.DOTALL) 
    if match:
        res_dict['#and_rw'] = int(match.group(1))
    
    key = 'lev'
    pattern = rf"{key}\s*=\s*(\d+)"
    match = re.search(pattern, truncated_text, re.DOTALL) 
    if match:
        res_dict['#lev_rw'] = match.group(1)

    return res_dict

with open(file_path, 'r') as file:
    resRewrite = []
    resRefactor = []
    resResub = [] 
    
    for line in file:  
        original_res = parseOriginalRes(line)
        
        # 如果 original_res 为空（没匹配到基本信息），则跳过该行
        if not original_res:
            continue

        if "rewrite" in line:
            rewrite_res = parseRewriteRes(line) 
            if rewrite_res:
                combined_res = {**original_res, **rewrite_res}
                resRewrite.append(combined_res)
            # 如果没有 rewrite 统计信息，可以选择不添加或者只添加 original

        if "refactor" in line:
            refactor_res = parseRewriteRes(line) # 注意：你原代码这里调用的也是 parseRewriteRes，确认逻辑是否一致
            if refactor_res:
                combined_res = {**original_res, **refactor_res}
                resRefactor.append(combined_res)
        
        if "resub" in line:
            resub_res = parseRewriteRes(line) # 同上
            if resub_res:
                combined_res = {**original_res, **resub_res}
                resResub.append(combined_res)
                
    if resRewrite:
        dfRewrite = pd.DataFrame(resRewrite)
        dfRewrite = dfRewrite.sort_values(by='#and', ascending=True)

        # --- 修改点 2: 强制将 design 列移动到第一列 ---
        cols = dfRewrite.columns.tolist()
        if 'design' in cols:
            # 将 design 从列表中移除并插入到头部
            cols.remove('design')
            cols = ['design'] + cols
            dfRewrite = dfRewrite[cols]

        path = file_path.split('/')[-1]
        output_filename = './exp-analysis/' + path.split('.')[0] + '_rewrite.csv'
        print(f"Saving to {output_filename}")
        dfRewrite.to_csv(output_filename, index=False)
    else:
        print("No rewrite data found.")     