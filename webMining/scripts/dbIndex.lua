local initdb = function()
  local db = sqlite3.open("index.db")
  db:exec("pragma foreign_keys = '1';");
  return db
end

local ddl = function(db)
  -- [=[
  -- drop
  db:exec([[
    drop table wordcount;
    drop table word;
    drop table document;
  ]])
  -- ]=]
  
  -- create
  db:exec([[
    create table document (
      id integer primary key,
      uri varchar unique not null
    );
    
    create table word (
      id integer primary key,
      word varchar unique not null
    );
    
    create table wordcount (
      document integer not null,
      word integer not null,
      count integer default 0,
      primary key(document, word),
      foreign key(word) references word(id),
      foreign key(document) references document(id)
    );
  ]])
  print(db:errmsg())
  
  -- delete
  --[=[
  db:exec([[
    delete from period;
    delete from region;
    delete from document;
  ]])
  --]=]
end

local getDocumentId = function(db, uri)
  local stmt = db:prepare([[
    insert into document (uri) values (:uri);
  ]])
  stmt:bind_values(uri)
  stmt:step()
  local err = stmt:finalize()
  if err == sqlite3.OK then
    return db:last_insert_rowid()
  elseif db:errcode() == sqlite3.CONSTRAINT then
    for id in db:urows("select id from document where uri = '" .. uri .. "';") do
      return id
    end
  end
end

local getWordId = function(db, word)
  local stmt = db:prepare([[
    insert into word (word) values (:word);
  ]])
  stmt:bind_values(word)
  stmt:step()
  local err = stmt:finalize()
  if err == sqlite3.OK then
    return db:last_insert_rowid()
  elseif db:errcode() == sqlite3.CONSTRAINT then
    for id in db:urows("select id from word where word = '" .. word .. "';") do
      return id
    end
  end
end

local cleanDocWordCount = function(db, docId)
  local stmt = db:prepare([[
    delete from wordcount where document = :docId;
  ]])
  stmt:bind_values(docId)
  stmt:step()
  local err = stmt:finalize()
end

local insertWordCount = function(db, docId, wordId, count)
  local stmt = db:prepare([[
    insert into wordcount (document, word, count) values (:docId, :wordId, :count);
  ]])
  stmt:bind_values(docId, wordId, count)
  stmt:step()
  local err = stmt:finalize()
end

local docVector = function(db, docId)
  -- total number of indexed documents
  local n = 0
  for c in db:urows("select count(*) from document;") do
    n = c
    break
  end

  -- word count in docId
  local wordlist = ""  
  local tf = {}
  local maxf = 1
  for word, count in db:urows("select word, document from wordcount where document = "..docId..";") do
    tf[word] = count
    if wordlist == "" then
      wordlist = word
    else
      wordlist = wordlist .. "," .. word
    end
    if count > maxf then
      maxf = count
    end
  end

  -- document count by word
  local df = {}
  for word, count in db:urows("select word,count(*)  from wordcount where word in ("..wordlist..") group by word;") do
    df[word] = count
  end

  -- normalized word count
  for word, count in pairs(tf) do
    tf[word] = df[word] / maxf
  end
  
  -- inverse document count
  local idf = {}
  for word, count in pairs(df) do
    idf[word] = math.log(n/df[word])
  end
  
  -- final word weight
  local w = {}
  for word, count in pairs(idf) do
    w[word] = tf[word] * idf[word]
  end
  
  return w
end

local queryVector = function(db, wordIdList)
  -- total number of indexed documents
  local n = 0
  for c in db:urows("select count(*) from document;") do
    n = c
    break
  end
  
  -- document count by word
  local df = {}
  for word, count in db:urows("select word,count(*)  from wordcount where word in ("..wordIdList..") group by word;") do
    df[word] = count
  end

  -- inverse document count
  local w = {}
  for word, count in pairs(df) do
    w[word] = math.log(n/df[word])
  end

  return w
end

local cosineDistance = function(query, doc)
  local a,b
  if #query < #doc then
    a = query
    b = doc
  else
    b = query
    a = doc
  end
  
  local dotProd = 0
  local sizeA = 0
  local sizeB = 0
  for word,weight in pairs(a) do
    sizeA = sizeA + weight
    if b[word] ~= nil then
      dotProd = dotProd  + (b[word] * weight)
    end
  end
  
  for word, weight in pairs(b) do
    sizeB = sizeB + weight
  end
  
  local score = dotProd/(sizeA * sizeB)
  print (score)
  
  return score
end

local indexWords = function(db, uri, wordCount)
  local docId = getDocumentId(db, uri)
  cleanDocWordCount(db, docId)
  for word,count in pairs(wordCount) do
    local wordId = getWordId(db, word)
    insertWordCount(db, docId, wordId, count)
  end
end

tableAccents = {
  ["à"] = "a", ["á"] = "a",  ["â"] = "a",  ["ã"] = "a", ["ä"] = "a",
  ["ç"] = "c",
  ["è"] = "e", ["é"] = "e", ["ê"] = "e", ["ë"] = "e",
  ["ì"] = "i", ["í"] = "i", ["î"] = "i", ["ï"] = "i",
  ["ñ"] = "n",
  ["ò"] = "o", ["ó"] = "o", ["ô"] = "o", ["õ"] = "o", ["ö"] = "o",
  ["ù"] = "u", ["ú"] = "u", ["û"] = "u", ["ü"] = "u",
  ["ý"] = "y", ["ÿ"] = "y",
  ["À"] = "A", ["Á"] = "A", ["Â"] = "A", ["Ã"] = "A", ["Ä"] = "A",
  ["Ç"] = "C",
  ["È"] = "E", ["É"] = "E", ["Ê"] = "E", ["Ë"] = "E",
  ["Ì"] = "I", ["Í"] = "I", ["Î"] = "I", ["Ï"] = "I",
  ["Ñ"] = "N",
  ["Ò"] = "O", ["Ó"] = "O", ["Ô"] = "O", ["Õ"] = "O", ["Ö"] = "O",
  ["Ù"] = "U", ["Ú"] = "U", ["Û"] = "U", ["Ü"] = "U",
  ["Ý"] = "Y"
 }
 
function string.stripAccents(input)
  local output = ""
   
  for ch in string.gmatch(input, "([%z\1-\127\194-\244][\128-\191]*)") do
    if tableAccents[ch] ~= nil then
      output = output..tableAccents[ch]
    else
      output = output..ch
    end
  end
  return output
end

local visit = function (dom, node)
  if node:isText() then
    local s = node:toString()
    s = string.stripAccents(s)
    s = string.lower(s)
    for w in string.gmatch(s, "%w+") do
      if wordCount[w] == nil then
        wordCount[w] = 0
      end
      wordCount[w] = wordCount[w] + 1
    end
  end
  return 1
end

indexDocument = function(uri)
  local dom = DOM.new(uri)
  dom:setVisitFunction(visit)
  wordCount = {}
  dom:traverse(0, dom:body()) -- 1 = breadth first; 0 = depth first
  
  indexWords(db, dom:getURI(), wordCount)
end

processQuery = function(db, query)
  -- make word unique
  local queryWords = {}
  for word in string.gmatch(query, "%w+") do
    table.insert(queryWords,string.stripAccents(word))
  end
  
  local wordlist = ""
  for i,word in pairs(queryWords) do
    if wordlist == "" then
      wordlist = "\'"..word.."\'"
    else
      wordlist = wordlist .. "," .. "\'"..word.."\'"
    end
  end
  print(wordlist)
  
  -- convert word string do wordId
  local wordIdList = ""
  for wordId in db:urows("select id from word where word in ("..wordlist..");") do
    if wordIdList == "" then
      wordIdList = wordId
    else
      wordIdList = wordIdList .. "," .. wordId
    end
  end
  print(wordIdList)

  local qw = queryVector(db, wordIdList)
  
  for k,v in pairs(qw) do
    print(k,"=",v)
  end
  
  local dw = {}
  for docId in db:urows("select document from wordcount where word in (" .. wordIdList .. ");") do
    dw[docId] = docVector(db, docId)
  end

  docs = {}
  for docId, vec in pairs(dw) do
    table.insert(docs,{docId, cosineDistance(qw, dw[docId])})
  end
  
  table.sort(docs, function(a,b) return a[2] > b[2] end)
  
  return docs
end

db = initdb()
--ddl(db)