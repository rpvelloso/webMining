plotSequences = function(dsre,output,filename)
  local regions = dsre:regionCount()
  local tps = dsre:getTps()

  local f = io.open(filename..".plot.txt","w")
  if output == "file" then
    f:write("set term ",term["default"],CRLF)
  else
    f:write("set mouse",CRLF)
    f:write("set multiplot layout ",regions+1,",1",CRLF)
  end

  f:write("set output \"",filename,".tps",term[term["default"]],"\"",CRLF)
  f:write("set autoscale fix",CRLF)
  f:write("set style line 1 lc rgb \'#0060ad\' lt 1 lw 1 pt 7 ps 0.5",CRLF)
  f:write("plot '-' with linespoints ls 1 title 'Full TPS'",CRLF)
  for i=1,#tps do
        f:write(i-1,"\t",tps[i],CRLF)
  end
  f:write("e",CRLF)

  for i=1,regions do
    local dr = dsre:getDataRegion(i-1)
    local linReg = dr:getLinearRegression()
    tps = dr:getTps()
    local transform = dr:getTransform()
    if #tps then
      f:write("set output \"",filename,".region",i,term[term["default"]],"\"",CRLF)
      f:write("set autoscale fix",CRLF)
      f:write("set multiplot layout 2,1",CRLF)
      f:write("set style line 1 lc rgb \'#0060ad\' lt 1 lw 1 pt 7 ps 0.5",CRLF)
      f:write("plot ",linReg.a,"*x+",linReg.b," with lines title 'Linear regression','-' with linespoints ls 1 title \'Region ",i,"\'",CRLF)
      for j=1,#tps do
        f:write(j-1,"\t",tps[j],CRLF)
      end
      f:write("e",CRLF)
      f:write("set arrow from "..(dr:recordCount())..", graph 0 to "..(dr:recordCount())..", graph 1 nohead lt 1 lw "..(5).." lc rgb \'red\'",CRLF)
      f:write("plot '-' with linespoints ls 1 title \'Transform \'",CRLF)
      for i=1,#transform do
            f:write((i-1)*#tps/#transform,"\t",transform[i],CRLF)
      end
      f:write("e",CRLF)
      f:write("unset arrow",CRLF)
      f:write("unset multiplot",CRLF)
    end
  end
  f:write("quit",CRLF)
  f:close()
  os.execute(gnuplot.." "..filename..".plot.txt")
end
