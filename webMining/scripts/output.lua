displayResults = function(dsre,dir,filename) 
  local j=0
  local regions = dsre:regionCount()
  local outp = io.open(dir..filename,"w")

  outp:write("<pre> timestamp: ");
  outp:write(os.clock());
  outp:write("</pre><br/>\n");
  
  local tps = dsre:getTps()
  if #tps then
    outp:write("<style>table {border-collapse: collapse;} table, td, th {border: 1px solid black;}</style>")
    outp:write("<font face=courier><img src='",filename,".tps",term[term["default"]],"' /><br />",CRLF)
    outp:write("<textarea>",CRLF)
    outp:write(tps[1])
    for k=2,#tps do
      outp:write(",",tps[k])
    end
    outp:write("</textarea><br />")
  end
  for i=1,regions do
    local dr = dsre:getDataRegion(i-1)
    if dr:isContent() then
      outp:write("<font color=red><b>*** Content detected ***</b></font><br>",CRLF)
    end
    local rows = dr:recordCount()
    local cols = dr:recordSize()
    local linReg = dr:getLinearRegression()
    outp:write("<table><tr><th> region ",i,"</th><th> rows ",rows,"</th><th> cols ",cols,"</th></tr></table>",CRLF)
    
    if (rows > 0) and (cols > 0) then 
      outp:write("<table>",CRLF)
      print(rows, math.abs(linReg.a)*180.0/math.pi)
      for r=1,rows do
        outp:write("<tr>")
        local record = dr:getRecord(r-1)
        for c=1,cols do
          if record[c] then
            outp:write("<td>",record[c]:toString(),"</td>")
          else
            outp:write("<td>[filler]</td>")
          end
        end
        outp:write("</tr>",CRLF)
        j = j + 1
      end
      outp:write("</table><br />",CRLF)
    end
    
    local tps = dr:getTps()
    local transform = dr:getTransform()
    if #tps then
      outp:write("<img src='",filename,".region",i,term[term["default"]],"' /><br />",CRLF)
      outp:write(string.format("interval: [%d; %d], size: %d, angle: %.2f, score: %.2f<br/>",dr:getStartPos(),dr:getEndPos(),dr:size(),math.abs(linReg.a)*180.0/math.pi,dr:getScore()),CRLF)
      outp:write("<textarea>",CRLF)
      outp:write(tps[1])
      for k=2,#tps do
        outp:write(",",tps[k])
      end
      outp:write("</textarea>",CRLF)
      if #transform then
        outp:write("<textarea>",CRLF)
        outp:write(transform[1])
        for k=2,#transform do
          outp:write(",",transform[k])
        end
        outp:write("</textarea><br />",CRLF)
      end
    end
    outp:write(CRLF)
  end
  outp:write(regions," regions, ",j," records.",CRLF)
  outp:write("</font><hr/><br/>",CRLF)
  outp:close()
end

