initSQLite = function()
  db = sqlite3.open("dsre.db")
  db:exec("pragma foreign_keys = '1';");
  --[[
    drop table period;\
    drop table region;\
    drop table document;\
  --]]
  
  --[=[
  db:exec([[
    create table document (id integer primary key, uri varchar unique);
    create table region (id integer primary key, document integer not null, beginpos integer, endpos integer, tps varchar, unique(document, beginpos, endpos), foreign key(document) references document(id));
    create table period (id integer primary key, region integer not null, type integer, value float, count integer, unique(region, type), foreign key(region) references region(id));
    ]]);
  print(db:errmsg());
  --]=]
  
  db:exec([[
    delete from period;
    delete from region;
    delete from document;
  ]]);
  return db
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

db = initSQLite()
