import os
import json

path = "/mnt/30.133_xintang/Data/Crypto/TardisSource"

# 确保路径存在
if not os.path.exists(path):
    print(f"The path {path} does not exist.")
else:
    # 获取路径下的所有文件和文件夹
    directory_contents = os.listdir(path)

    # 字典用于存储符合条件的目录名
    symbols = {"symbols": []}

    # 遍历目录内容，提取符合条件的条目
    for item in directory_contents:
        if item.startswith("binance_usd_"):
            # 提取货币对名称
            currency_pair = item.split("binance_usd_")[-1]
            symbols["symbols"].append(currency_pair)

    # 将结果写入JSON文件
    with open('symbols.json', 'w') as json_file:
        json.dump(symbols, json_file, indent=4)

    print("JSON file has been created with the extracted symbols.")
