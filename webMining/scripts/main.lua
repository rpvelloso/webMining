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

if #args > 4 then
  processFile(args[5])
  do return end
end

processTestBed("../../datasets/clustvx")
do return end
processTestBed("../../datasets/tpsf")
processTestBed("../../datasets/yamada")
-- [[
processTestBed("../../datasets/zhao3")
processTestBed("../../datasets/TWEB_TB2")
processTestBed("../../datasets/TWEB_TB3")
processTestBed("../../datasets/alvarez")
processTestBed("../../datasets/wien")
processTestBed("../../datasets/zhao1")
processTestBed("../../datasets/zhao2")
processTestBed("../../datasets/trieschnigg1")
processTestBed("../../datasets/trieschnigg2")
-- ]]
