CRLF = "\n"
gnuplot = "c:\\Progra~1\\gnuplot\\bin\\gnuplot.exe"
--gnuplot = "/usr/bin/gnuplot"

term = {}
term["png"] = ".png"
term["postscript"]=".ps"
term["svg"]=".svg"
term["default"]="svg"

initSQLite = function()
  db = sqlite3.open("dsre.db")
  db:exec("pragma foreign_keys = '1';");
  --[[
    drop table period;\
    drop table region;\
    drop table document;\
  --]]
  --[[
  db:exec("\
    create table document (id integer primary key, uri varchar unique);\
    create table region (id integer primary key, document integer not null, beginpos integer, endpos integer, tps varchar, unique(document, beginpos, endpos), foreign key(document) references document(id));\
    create table period (id integer primary key, region integer not null, type integer, value float, count integer, unique(region, type), foreign key(region) references region(id));\
    ");
  print(db:errmsg());
  --]]
  db:exec([[
    delete from period;
    delete from region;
    delete from document;
  ]]);
  return db
end

tps2String = function(tps)
  ret = ''
  if #tps > 0 then
    ret = ret..tps[1]
    if #tps > 1 then
      for i=2,#tps do
        ret = ret..','..tps[i] 
      end
    end
  end
  return ret
end

queryDB = function()
  print("URI;REGION;FFT;DCT;MDCT")
  for r in db:nrows([[
    select d.uri, p0.region, p0.value value0, p1.value value1, p2.value value2 
    from document d, region r, period p0, period p1, period p2 
    where 
    d.id = r.document and r.id = p0.region and p0.region = p1.region and p0.region = p2.region and 
    p0.type = 0 and p1.type = 1 and p2.type = 2 and 
    not (abs(p0.value - p1.value) < 2 and abs(p0.value - p2.value)< 2);
  ]]) do
  --select d.uri, p0.region, p0.value value0, p1.value value1, p2.value value2, max(abs(p0.value - p1.value),abs(p0.value-p2.value),abs(p1.value-p2.value)) maxdiff from document d, region r, period p0, period p1, period p2 where d.id=r.document and r.id = p0.region and p0.region = p1.region and p0.region = p2.region and p0.type = 0 and p1.type = 1 and p2.type = 2 and not (abs(p0.value - p1.value) < 2 and abs(p0.value - p2.value)< 2) order by maxdiff desc;
    print(string.format("%s;%d;%.2f;%.2f;%.2f",r.uri,r.region,r.value0,r.value1,r.value2))
  end
  print("URI;REGION;BEGIN;END;TYPE;VALUE")
  for r in db:nrows([[
    select d.uri, r.id region, r.beginpos, r.endpos, p.type, p.value 
    from document d, region r, period p
    where d.id = r.document and r.id = p.region
    order by d.uri, r.beginpos, p.type;
  ]]) do
    print(string.format("%s;%d;%d;%d;%d;%.2f",r.uri,r.region,r.beginpos,r.endpos,r.type,r.value))
  end
end

insertDocument = function(uri)
  stmt = db:prepare([[
  insert into document (uri) values (:uri);
  ]])
  stmt:bind_values(uri)
  stmt:step("insert into document (uri) values ('" .. uri .. "');")
  err = stmt:finalize()
  if err == sqlite3.OK then
    return db:last_insert_rowid()
  elseif db:errcode() == sqlite3.CONSTRAINT then
    for id in db:urows("select id from document where uri = '" .. uri .. "';") do
      return id
    end
  end
  print("SQLITE error code " .. db:errcode() .. ": '" .. db:errmsg() .. "'")
  os.exit()
end

updateRegion = function(id, tps)
  stmt = db:prepare([[
    update region set tps = :tps where id = :id
  ]])
  stmt:bind_values(tps, id)
  stmt:step()
  err = stmt:finalize()
  if err ~= sqlite3.OK then
    print("SQLITE error code " .. db:errcode() .. ": '" .. db:errmsg() .. "'")
    os.exit()
  end
end

insertRegion = function(values)
  stmt = db:prepare([[
    insert into region (document, beginpos, endpos, tps) 
    values (:document, :beginpos, :endpos, :tps);
  ]])
  stmt:bind_names(values)
  stmt:step()
  err = stmt:finalize()
  if err == sqlite3.OK then
    return db:last_insert_rowid()
  elseif db:errcode() == sqlite3.CONSTRAINT then
    for id in db:urows("select id from region where document = "..values["document"].." and beginpos = "..values["beginpos"].." and endpos = "..values["endpos"]..";") do
      updateRegion(id, values["tps"])
      return id
    end
  end
  print("SQLITE error code " .. db:errcode() .. ": '" .. db:errmsg() .. "'")
  os.exit()
end

updatePeriod = function(id, value, count)
  stmt = db:prepare([[
    update period set value = :value, count = :count where id = :id
  ]])
  stmt:bind_values(value, count, id)
  stmt:step()
  err = stmt:finalize()
  if err ~= sqlite3.OK then
    print("SQLITE error code " .. db:errcode() .. ": '" .. db:errmsg() .. "'")
    os.exit()
  end
end

insertPeriod = function(values)
  stmt = db:prepare([[
    insert into period (region, type, value, count)
    values (:region, :type, :value, :count)
  ]])
  stmt:bind_names(values)
  stmt:step()
  err = stmt:finalize()
  if err == sqlite3.OK then
    return db:last_insert_rowid()
  elseif db:errcode() == sqlite3.CONSTRAINT then
    for id in db:urows("select id from period where region = "..values["region"].." and type = "..values["type"]..";") do
      updatePeriod(id, values["value"], values["count"])
      return id
    end
  end
  print("SQLITE error code " .. db:errcode() .. ": '" .. db:errmsg() .. "'")
  os.exit()
end

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
    outp:write("<table><tr><th> region ",i,"</th><th> rows ",rows,"</th><th> cols ",cols,"</th></tr></table>",CRLF)
    
    if (rows > 0) and (cols > 0) then 
      outp:write("<table>",CRLF)
      print(rows)
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
    local linReg = dr:getLinearRegression()
    if #tps then
      outp:write("<img src='",filename,".region",i,term[term["default"]],"' /><br />",CRLF)
      outp:write(string.format("interval: [%d; %d], size: %d, angle: %.2f, score: %.2f<br/>",dr:getStartPos(),dr:getEndPos(),dr:size(),math.atan(math.abs(linReg.a))*180/math.pi,dr:getScore()),CRLF)
      outp:write("<textarea>",CRLF)
      outp:write(tps[1])
      for k=2,#tps do
        outp:write(",",tps[k])
      end
      outp:write("</textarea><br />",CRLF)
    end
    outp:write(CRLF)
  end
  outp:write(regions," regions, ",j," records.",CRLF)
  outp:write("</font><hr/><br/>",CRLF)
  outp:close()
end

plotSequences = function(dsre,output,filename)
  local regions = dsre:regionCount()
  local tps = dsre:getTps()

  f = io.open(filename..".plot.txt","w")
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
    if #tps then
      f:write("set output \"",filename,".region",i,term[term["default"]],"\"",CRLF)
      f:write("set autoscale fix",CRLF)
      f:write("set style line 1 lc rgb \'#0060ad\' lt 1 lw 1 pt 7 ps 0.5",CRLF)
      f:write("plot ",linReg.a,"*x+",linReg.b," with lines title 'Linear regression','-' with linespoints ls 1 title \'Region ",i,"\'",CRLF)
      for j=1,#tps do
        f:write(j-1,"\t",tps[j],CRLF)
      end
      f:write("e",CRLF)
    end
  end
  f:write("unset multiplot",CRLF)
  f:write("quit",CRLF)
  f:close()
  os.execute(gnuplot.." "..filename..".plot.txt")
end

saveDB = function(dom, dsre)
  local regionCount = dsre:regionCount()
  local docID = insertDocument(dom:getURI())
  for r=1,regionCount do
    local dr = dsre:getDataRegion(r-1)
    
    local drID = insertRegion{
      document = docID, 
      beginpos = dr:getStartPos(), 
      endpos = dr:getEndPos(), 
      tps = tps2String(dr:getTps())
    }
    
    local perID = insertPeriod{
      region = drID, 
      type = dr:getPeriodEstimator(), 
      value = dr:getEstPeriod(),
      count = dr:recordCount()
    }
  end
end

processTestBed = function(dir, generateOutput)
  local t, popen = {}, io.popen
  generateOutput = generateOutput or 1

  for filename in popen('ls -a "'..dir..'"/*.htm*'):lines() do
    local d, fn, ext = filename:match("(.-)([^\\/]-%.?([^%.\\/]*))$")
    local output = d.."srde/"..fn
    
    print(string.format("Loading DOM tree: %s",filename),CRLF)
    local dom = DOM.new(filename)
    local dsre = DSRE.new()
    --dsre:setPeriodEstimator(1)
    
    --print("Extracting records.")
    local start = os.clock()
    dsre:extract(dom)
    print(string.format("elapsed time: %.2f",os.clock() - start),CRLF)
    
    if generateOutput == 1 then
      --print("Outputting results.")
      displayResults(dsre,d.."srde/",fn)
      
      --print("Plotting graphs.")
      plotSequences(dsre,"file",output)
    end
    saveDB(dom,dsre)
  end
end

processFile = function(filename)
    print(string.format("Loading DOM tree: %s",filename),CRLF)
    local dom = DOM.new(filename)
    local dsre = DSRE.new()
    --local dsre = CVSRE.new()
    
    print("Extracting records.")
    local start = os.clock()
    --dsre:setPeriodEstimator(0)
    dsre:extract(dom)
    print(string.format("elapsed time: %.2f",os.clock() - start),CRLF)
    
    print("Outputting results.")
    displayResults(dsre,"./","output.html")
    
    print("Plotting graphs.")
    plotSequences(dsre,"file","output.html")
    
    --dom:printHTML()
    --dsre:printTps()
end

db = initSQLite()
--queryDB()
--do return end

if #args > 4 then
  processFile(args[5])
  do return end
end

processTestBed("../../datasets/clustvx")
processTestBed("../../datasets/tpsf")
--do return end
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
