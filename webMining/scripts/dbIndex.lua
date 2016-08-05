searchEngine = {
  db = nil
}

function searchEngine:initdb(dbfile)
  self.db = sqlite3.open(dbfile)
  self.db:exec("pragma foreign_keys = '1';");
  return self.db
end

function searchEngine:ddl()
  -- [=[
  -- drop
  self.db:exec([[
    drop table wordcount;
    drop table word;
    drop table document;
  ]])
  -- ]=]
  
  -- create
  self.db:exec([[
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
  print(self.db:errmsg())
  
  -- delete
  --[=[
  db:exec([[
    delete from period;
    delete from region;
    delete from document;
  ]])
  --]=]
end

function searchEngine:getDocumentId(uri)
  local stmt = self.db:prepare([[
    insert into document (uri) values (:uri);
  ]])
  stmt:bind_values(uri)
  stmt:step()
  local err = stmt:finalize()
  if err == sqlite3.OK then
    return self.db:last_insert_rowid()
  elseif self.db:errcode() == sqlite3.CONSTRAINT then
    for id in self.db:urows("select id from document where uri = '" .. uri .. "';") do
      return id
    end
  end
  return nil
end

function searchEngine:getWordId(word)
  local stmt = self.db:prepare([[
    insert into word (word) values (:word);
  ]])
  stmt:bind_values(word)
  stmt:step()
  local err = stmt:finalize()
  if err == sqlite3.OK then
    return self.db:last_insert_rowid()
  elseif self.db:errcode() == sqlite3.CONSTRAINT then
    for id in self.db:urows("select id from word where word = '" .. word .. "';") do
      return id
    end
  end
  return nil
end

function searchEngine:cleanDocWordCount(docId)
  local stmt = self.db:prepare([[
    delete from wordcount where document = :docId;
  ]])
  stmt:bind_values(docId)
  stmt:step()
  local err = stmt:finalize()
end

function searchEngine:insertWordCount(docId, wordId, count)
  local stmt = self.db:prepare([[
    insert into wordcount (document, word, count) values (:docId, :wordId, :count);
  ]])
  stmt:bind_values(docId, wordId, count)
  stmt:step()
  local err = stmt:finalize()
end

function searchEngine:docVector(n, docId)
  -- word count in docId
  local wordIdList = ""  
  local tf = {}
  local maxf = 1
  for word, count in self.db:urows("select word, document from wordcount where document = "..docId..";") do
    tf[word] = count
    if wordIdList == "" then
      wordIdList = word
    else
      wordIdList = wordIdList .. "," .. word
    end
    if count > maxf then
      maxf = count
    end
  end

  -- normalized word count
  for word, count in pairs(tf) do
    tf[word] = tf[word] / maxf
  end
  
  -- document count by word
  local df = {}
  for word, count in self.db:urows("select word,count(*)  from wordcount where word in ("..wordIdList..") group by word;") do
    df[word] = count
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

function searchEngine:queryVector(n, wordIdList)
  -- document count by word
  local df = {}
  for word, count in self.db:urows("select word,count(*)  from wordcount where word in ("..wordIdList..") group by word;") do
    df[word] = count
  end

  -- inverse document count
  local w = {}
  for word, count in pairs(df) do
    w[word] = math.log(n/df[word])
  end

  return w
end

function searchEngine:cosineDistance(query, doc)
  --[=[
  io.write("query vector=")
  for k,v in pairs(query) do
    if v>0 then
      io.write(string.format("%d=%f, ",k,v))
    end
  end
  print()

  io.write("  doc vector=")
  for k,v in pairs(doc) do
    if v>0 then
      io.write(string.format("%d=%f, ",k,v))
    end
  end
  print()
  --]=]

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
  
  local score = 0
  if sizeA > 0 and sizeB > 0 then
    score = dotProd/(sizeA * sizeB)
  end
  
  return score
end

function searchEngine:indexWords(uri, wordCount)
  local docId = self:getDocumentId(uri)
  local wc = 0
  self:cleanDocWordCount(docId)
  for word,count in pairs(wordCount) do
    local wordId = self:getWordId(word)
    self:insertWordCount(docId, wordId, count)
    wc = wc + 1
  end
  return wc
end

searchEngine.tableAccents = {
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
 
function searchEngine.stripAccents(input)
  local output = ""
   
  for ch in string.gmatch(input, "([%z\1-\127\194-\244][\128-\191]*)") do
    if searchEngine.tableAccents[ch] ~= nil then
      output = output..searchEngine.tableAccents[ch]
    else
      output = output..ch
    end
  end
  return output
end

function searchEngine.visitDOMNode(dom, node)
  if node:isText() then
    local s = node:toString()
    s = searchEngine.stripAccents(s)
    s = string.lower(s)
    for w in string.gmatch(s, "%w+") do
      if searchEngine.wordCount[w] == nil then
        searchEngine.wordCount[w] = 0
      end
      searchEngine.wordCount[w] = searchEngine.wordCount[w] + 1
    end
  end
  return 1
end

function searchEngine:indexDocument(uri)
  local dom = DOM.new(uri)
  dom:setVisitFunction(searchEngine.visitDOMNode)
  searchEngine.wordCount = {}
  dom:traverse(0, dom:body()) -- 1 = breadth first; 0 = depth first
  
  return self:indexWords(dom:getURI(), searchEngine.wordCount)
end

function searchEngine:processQuery(query)
  -- total number of indexed documents
  local n = 0
  for c in self.db:urows("select count(*) from document;") do
    n = c
    break
  end
  
  -- make word unique and strip accents
  local queryWords = {}
  for word in string.gmatch(query, "%w+") do
    table.insert(queryWords,searchEngine.stripAccents(word))
  end
  
  local wordList = ""
  for i,word in pairs(queryWords) do
    if wordList == "" then
      wordList = "\'"..word.."\'"
    else
      wordList = wordList .. "," .. "\'"..word.."\'"
    end
  end
  
  -- convert word string do word id
  local wordIdList = ""
  for wordId in self.db:urows("select id from word where word in ("..wordList..");") do
    if wordIdList == "" then
      wordIdList = wordId
    else
      wordIdList = wordIdList .. "," .. wordId
    end
  end

  local qw = self:queryVector(n, wordIdList)
  
  local dw = {}
  for docId in self.db:urows("select document from wordcount where word in (" .. wordIdList .. ");") do
    dw[docId] = self:docVector(n, docId)
  end

  local docs = {}
  for docId, vec in pairs(dw) do
    table.insert(docs,{docId, self:cosineDistance(qw, vec)})
  end
  
  table.sort(docs, 
  function(a,b) 
    return a[2] > b[2] 
  end)
  
  return docs
end

function searchEngine:getDocumentURI(docId)
  for uri in self.db:urows("select uri from document where id = "..docId..";") do
    return uri
  end
  return nil
end

searchEngine:initdb("index.db")
--searchEngine:ddl(db)