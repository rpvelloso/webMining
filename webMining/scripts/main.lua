gnuplot = "c:\\Progra~2\\gnuplot\\bin\\gnuplot.exe"
--gnuplot = "/usr/bin/gnuplot"
CRLF = "\n"

term = {}
term["png"] = ".png"
term["postscript"]=".ps"
term["svg"]=".svg"

-- default terminal type (vector graphics)
term["default"]="svg"

--loadfile("../scripts/db.lua")()
loadfile("../scripts/plot.lua")()
loadfile("../scripts/output.lua")()
loadfile("../scripts/testbed.lua")()

--queryDB()
--do return end

minPSD = 11
--minCV = 0.30

--driver = webDriver.fireFox

if driver ~= nil then
  driver:newSession()
end

if #args > 4 then
  processFile(args[5])
  --driver:go(args[5])
  --processUrl(driver:getCurrentURL(), driver:getPageSource(), "./", "output.html")
  do return end
end


processTestBed("../../datasets/top")
processTestBed("../../datasets/clustvx")
processTestBed("../../datasets/tpsf")
processTestBed("../../datasets/lightExtractorExperiment")
processTestBed("../../datasets/zhao3")
processTestBed("../../datasets/yamada")
do return end
processTestBed("C:\\Users\\rvelloso\\workspace\\datasets\\top")
processTestBed("C:\\Users\\rvelloso\\workspace\\datasets\\clustvx")
processTestBed("C:\\Users\\rvelloso\\workspace\\datasets\\yamada")
processTestBed("../../datasets/zhao3")
-- [[
processTestBed("../../datasets/trieschnigg2")
processTestBed("../../datasets/alvarez")
processTestBed("../../datasets/TWEB_TB2")
processTestBed("../../datasets/TWEB_TB3")
processTestBed("../../datasets/wien")
processTestBed("../../datasets/zhao1")
processTestBed("../../datasets/zhao2")
processTestBed("../../datasets/trieschnigg1")
-- ]]
