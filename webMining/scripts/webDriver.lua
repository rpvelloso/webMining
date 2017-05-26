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

local driver = webDriver.fireFox

if #args > 4 then
  local url = args[5]
  driver:newSession()
  driver:go(url)
  --local ret = driver:executeScriptFromFile('C:\\Users\\rvelloso\\Desktop\\tmp.js',true)
  --print('return from script: ', ret)
  print('session: ', driver:getSession())
  local html = driver:getPageSource()
  driver:takeScreenshot("screenshot")
  print('url: ', driver:getCurrentURL())
  processUrl(url, html)
  driver:deleteSession()
  do return end
end

