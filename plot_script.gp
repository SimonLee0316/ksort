# 設置輸出圖片格式和文件名
set terminal pngcairo enhanced font "arial,12"
set output "time_analysis.png"

# 設置圖表標題和軸標籤
set title "time analysis"
set xlabel "elements"
set ylabel "exectime (nanoseconds)"

# 設置數據分隔符
set datafile separator ','

# 繪製折線圖
plot "output.csv" using 1:2 with linespoints linewidth 2 title "Timsort", \
     "output.csv" using 1:3 with linespoints linewidth 2 title "linuxsort", \
     "output.csv" using 1:4 with linespoints linewidth 2 title "Qsort"