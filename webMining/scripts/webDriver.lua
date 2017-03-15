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

local driver = webDriver.chrome

if #args > 4 then
  local url = args[5]
  driver:newSession()
  driver:go(url)
  local ret = driver:executeScript("return document.title;",false)
  print('return from script: ', ret)
  local html = driver:getPageSource()
  driver:takeScreenshot("screenshot")
  --processUrl(url, html)
  driver:deleteSession()
  do return end
end

