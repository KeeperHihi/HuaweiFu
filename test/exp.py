import os
import math
import matplotlib.pyplot as plt
import numpy as np
import matplotlib.cm as cm

def main():
    # 读取初始参数
    T, M, N, V, G, K = map(int, input().split())
    
    # 初始化数据结构
    operations = {
        'delete': [[] for _ in range(M+1)],
        'write': [[] for _ in range(M+1)],
        'read': [[] for _ in range(M+1)]
    }

    # 读取数据
    for op in ['delete', 'write', 'read']:
        for i in range(1, M+1):
            operations[op][i] = list(map(int, input().split()))

    # 创建各tag目录
    for tag in range(1, M+1):
        os.makedirs(f"tag_{tag}", exist_ok=True)

    # 生成单个tag的独立图表
    def generate_single_plots():
        for tag in range(1, M+1):
            time_slices = list(range(1, len(operations['delete'][tag])+1))
            
            for op_name in operations:
                plt.figure(figsize=(10, 6))
                plt.plot(time_slices, operations[op_name][tag], 
                        marker='o', 
                        color=get_color(op_name),
                        linewidth=2)
                
                plt.title(f'{op_name.capitalize()} Requests for Tag {tag}')
                plt.xlabel('Time Slice')
                plt.ylabel('Count')
                plt.grid(True)
                
                save_path = os.path.join(f"tag_{tag}", f"{op_name}.png")
                plt.savefig(save_path, bbox_inches='tight')
                plt.close()

    # 生成全局对比图表
    def generate_global_comparison():
        for op_name in operations:
            # 创建大画布
            cols = min(5, M)  # 每行最多5个子图
            rows = math.ceil(M / cols)
            
            fig = plt.figure(figsize=(cols*4, rows*3))  # 动态调整画布尺寸
            fig.suptitle(f"{op_name.capitalize()} Operations Across All Tags", 
                         fontsize=14, y=1.02)
            
            for tag in range(1, M+1):
                ax = fig.add_subplot(rows, cols, tag)
                time_slices = list(range(1, len(operations[op_name][tag])+1))
                
                # 绘制当前tag的数据
                ax.plot(time_slices, operations[op_name][tag],
                       color=get_color(op_name),
                       marker='o',
                       markersize=4,
                       linewidth=1.5)
                
                # 子图装饰
                ax.set_title(f"Tag {tag}", fontsize=8)
                ax.tick_params(axis='both', labelsize=6)
                ax.grid(True, alpha=0.3)
                
                # 仅最外侧显示坐标标签
                if tag > (rows-1)*cols:
                    ax.set_xlabel("Time", fontsize=7)
                if tag % cols == 1:
                    ax.set_ylabel("Count", fontsize=7)

            # 调整布局并保存
            plt.tight_layout()
            plt.savefig(f"global_{op_name}_comparison.png", dpi=150)
            plt.close()
            
    # 绘制所有tag的读取操作在同一张图上
    def plot_all_tags_reads():
        plt.figure(figsize=(15, 10))
        
        # 获取实际的时间片数量
        time_slices = list(range(1, len(operations['read'][1])+1))
        
        # 使用不同颜色方案
        colors = cm.rainbow(np.linspace(0, 1, M))
        
        # 线型选项
        line_styles = ['-', '--', '-.', ':']
        
        for i, tag in enumerate(range(1, M+1)):
            plt.plot(time_slices, operations['read'][tag], 
                    label=f"Tag {tag}",
                    color=colors[i],
                    linestyle=line_styles[i % len(line_styles)],
                    linewidth=1.5)
        
        plt.title('Read Operations for All Tags')
        plt.xlabel('Time Slice')
        plt.ylabel('Count')
        plt.legend(loc='best', fontsize='small', ncol=2)
        plt.grid(True)
        plt.tight_layout()
        plt.savefig("all_tags_read_comparison.png", dpi=150, bbox_inches='tight')
        plt.close()

    # 分析每个时间片中读取最多的tag
    def analyze_top_tags_per_time_slice():
        # 获取实际的时间片数量
        time_slices = len(operations['read'][1])
        results = []
        
        for t in range(1, time_slices+1):
            # 收集每个tag在当前时间片的读取量
            tag_reads = []
            for tag in range(1, M+1):
                tag_reads.append((tag, operations['read'][tag][t-1]))
            
            # 按读取量排序（降序）
            tag_reads.sort(key=lambda x: x[1], reverse=True)
            
            # 获取前三名
            top_three = tag_reads[:3]
            
            # 检查差距是否过大
            if len(top_three) >= 2 and top_three[0][1] > 2 * top_three[1][1]:
                top_tags = [top_three[0][0]]
            elif len(top_three) >= 3 and top_three[1][1] > 2 * top_three[2][1]:
                top_tags = [tag for tag, _ in top_three[:2]]
            else:
                top_tags = [tag for tag, _ in top_three]
                
            results.append(top_tags)
        
        return results

    # 分析时间段
    def analyze_time_periods(top_tags_per_slice, period_size=6):
        periods = []
        
        for i in range(0, len(top_tags_per_slice), period_size):
            period = top_tags_per_slice[i:i+period_size]
            # 统计这个时间段内出现次数最多的tag
            tag_counts = {}
            for slice_tags in period:
                for tag in slice_tags:
                    tag_counts[tag] = tag_counts.get(tag, 0) + 1
            
            # 找出出现次数最多的tag
            sorted_tags = sorted(tag_counts.items(), key=lambda x: x[1], reverse=True)
            top_period_tags = [tag for tag, count in sorted_tags[:3]]
            
            periods.append({
                'time_range': f"{i+1}-{min(i+period_size, len(top_tags_per_slice))}",
                'top_tags': top_period_tags
            })
        
        return periods

    # 执行生成图表和分析
    generate_single_plots()
    generate_global_comparison()
    plot_all_tags_reads()
    
    top_tags_per_slice = analyze_top_tags_per_time_slice()
    print("每个时间片读取最多的Tag:")
    for i, tags in enumerate(top_tags_per_slice):
        print('{', end='')
        for j in range(len(tags)):
            print(tags[j], end=", ")
        print('},')
    
    # 分析不同粒度的时间段
    period_sizes = [6, 8, 12]
    for period_size in period_sizes:
        time_periods = analyze_time_periods(top_tags_per_slice, period_size)
        print(f"\n时间段划分(每{period_size}个时间片)主导Tag分析:")
        for period in time_periods:
            print(f"时间段 {period['time_range']}: {period['top_tags']}")

def get_color(op_name):
    return {
        'delete': '#e74c3c',
        'write': '#2ecc71',
        'read': '#3498db'
    }[op_name]

if __name__ == "__main__":
    main()
