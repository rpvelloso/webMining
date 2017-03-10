gnuplot = "c:\\Progra~2\\gnuplot\\bin\\gnuplot.exe"
--gnuplot = "/usr/bin/gnuplot"
CRLF = "\n"

term = {}
term["png"] = ".png"
term["postscript"]=".ps"
term["svg"]=".svg"

term["default"]="svg"

loadfile("../scripts/plot.lua")()
loadfile("../scripts/output.lua")()
loadfile("../scripts/testbed.lua")()

if #args > 4 then
  local url = args[5]
  webDriver:go(url)
  local html = webDriver:getPageSource()
  processUrl(url, html)
  do return end
end

