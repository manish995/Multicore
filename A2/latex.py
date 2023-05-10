data = []
for i in range(1, 5):
    line = []
    with open('prog' + str(i) + '_share', 'r') as f:
        lines = f.read().splitlines()[2:]
    for p in lines:
        line.append(p.split(' ')[4])
    data.append(line)

pre = "\\begin{table}[H]\n\\begin{tabular}{|c|c|}\n\\hline\n\\textbf{Trace Number} & \\textbf{1} & \\textbf{2} & \\textbf{3} & \\textbf{4} & \\textbf{5} & \\textbf{6} & \\textbf{7} & \\textbf{8} \\\\ \hline\n"
body = ""
for i in range(4):
    body += "\\textbf{" + str(i+1) + "} "    
    for j in range(8):
        body += "& \\textbf{" + str(data[i][j]) + "} "
    body += " \\\\ \hline\n"
end = "\\end{tabular}\n\\end{table}"
print(pre + body + end)