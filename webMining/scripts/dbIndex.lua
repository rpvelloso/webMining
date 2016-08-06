searchEngine = {
  db = nil
}

function searchEngine:initdb(dbfile)
  self.db = sqlite3.open(dbfile)
  --self.db:exec("pragma foreign_keys = '1';");
  return self.db
end

function searchEngine:ddl(drop)
  if drop == true then
    self.db:exec([[
      drop table wordcount;
      drop table word;
      drop table document;
    ]])
  end
  
  -- create
  self.db:exec([[
    create table document (
      id integer primary key,
      uri varchar unique not null
    );
    
    create table word (
      id integer primary key,
      word varchar unique not null,
      df integer not null default 0
    );
    
    create table wordcount (
      document integer not null,
      word integer not null,
      count integer default 0,
      primary key(document, word),
      foreign key(word) references word(id),
      foreign key(document) references document(id)
    );
    
    create index reversePK on wordcount (word, document);
  ]])
  
  -- delete
  --[=[
  db:exec([[
    delete from period;
    delete from region;
    delete from document;
  ]])
  --]=]

  if self.db:errcode() ~= sqlite3.OK then
    print(self.db:errmsg())
  end
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
    insert into word (word, df) values (:word, 0);
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
  local wordIdList = ""
  
  for word in self.db:urows("select word from wordcount where document = "..docId..";") do
    if wordIdList == "" then
      wordIdList = word
    else
      wordIdList = wordIdList..","..word
    end
  end
  self.db:exec("update word set df=df-1 where id in ("..wordIdList..");")
  
  local stmt = self.db:prepare([[
    delete from wordcount where document = :docId;
  ]])
  stmt:bind_values(docId)
  stmt:step()
  local err = stmt:finalize()
end

function searchEngine:docVector(n, docId)
  -- word count in docId
  local wordIdList = ""  
  local tf = {}
  local maxf = 1
  for word, count in self.db:urows("select word, count from wordcount where document = "..docId..";") do
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
  for word, count in self.db:urows("select id,df from word where id in ("..wordIdList..");") do
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
  for word, count in self.db:urows("select id,df from word where id in ("..wordIdList..");") do
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
  local docWordCountList = ""

  self:cleanDocWordCount(docId)
  for word,count in pairs(wordCount) do
    local wordId = self:getWordId(word)
    if docWordCountList == "" then
      docWordCountList = "("..docId..", "..wordId..", "..count..")"
    else
      docWordCountList = docWordCountList .. ", " .. "("..docId..", "..wordId..", "..count..")"
    end
    self.db:exec("update word set df=df+1 where id="..wordId..";")
    wc = wc + 1
  end

  if wc > 0 then
    local stmt = self.db:prepare("insert into wordcount (document, word, count) values "..docWordCountList..";")
    stmt:step()
    local err = stmt:finalize()
  end

  if self.db:errcode() ~= sqlite3.OK then
    print(self.db:errmsg())
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
 
 searchEngine.stopWords = {
 -- portuguese
  ["a"] = 0, ["ainda"] = 0, ["alem"] = 0, ["ambas"] = 0, ["ambos"] = 0, ["antes"] = 0,
  ["ao"] = 0, ["aonde"] = 0, ["aos"] = 0, ["apos"] = 0, ["aquele"] = 0, ["aqueles"] = 0,
  ["as"] = 0, ["assim"] = 0, ["com"] = 0, ["como"] = 0, ["contra"] = 0, ["contudo"] = 0,
  ["cuja"] = 0, ["cujas"] = 0, ["cujo"] = 0, ["cujos"] = 0, ["da"] = 0, ["das"] = 0,
  ["de"] = 0, ["dela"] = 0, ["dele"] = 0, ["deles"] = 0, ["demais"] = 0, ["depois"] = 0,
  ["desde"] = 0, ["desta"] = 0, ["deste"] = 0, ["dispoe"] = 0, ["dispoem"] = 0, ["diversa"] = 0,
  ["diversas"] = 0, ["diversos"] = 0, ["do"] = 0, ["dos"] = 0, ["durante"] = 0, ["e"] = 0,
  ["ela"] = 0, ["elas"] = 0, ["ele"] = 0, ["eles"] = 0, ["em"] = 0, ["entao"] = 0, 
  ["entre"] = 0, ["essa "] = 0, ["essas"] = 0, ["esse"] = 0, ["esses"] = 0, ["esta"] = 0, 
  ["estas"] = 0, ["este"] = 0, ["estes"] = 0, ["ha"] = 0, ["isso"] = 0, ["isto"] = 0, 
  ["logo"] = 0, ["mais"] = 0, ["mas"] = 0, ["mediante"] = 0, ["menos"] = 0, ["mesma"] = 0, 
  ["mesmas"] = 0, ["mesmo"] = 0, ["mesmos"] = 0, ["na"] = 0, ["nas"] = 0, ["nao"] = 0, 
  ["nas"] = 0, ["nem"] = 0, ["nesse"] = 0, ["neste"] = 0, ["nos"] = 0, ["o"] = 0, ["os"] = 0,
  ["ou"] = 0, ["outra"] = 0, ["outras"] = 0, ["outro"] = 0, ["outros"] = 0, ["pelas"] = 0,
  ["pelas"] = 0, ["pelo"] = 0, ["pelos"] = 0, ["perante"] = 0, ["pois"] = 0, ["por"] = 0,
  ["porque"] = 0, ["portanto"] = 0, ["proprio"] = 0, ["propios"] = 0, ["quais"] = 0,
  ["qual"] = 0, ["qualquer"] = 0, ["quando"] = 0, ["quanto  "] = 0, ["que"] = 0, ["quem"] = 0,
  ["quer"] = 0, ["se"] = 0, ["seja"] = 0, ["sem"] = 0, ["sendo"] = 0, ["seu"] = 0,
  ["seus"] = 0, ["sob"] = 0, ["sobre"] = 0, ["sua"] = 0, ["suas"] = 0, ["tal"] = 0, 
  ["tambem"] = 0, ["teu"] = 0, ["teus"] = 0, ["toda"] = 0, ["todas"] = 0, ["todo"] = 0,
  ["todos"] = 0, ["tua"] = 0, ["tuas"] = 0, ["tudo"] = 0, ["um"] = 0, ["uma"] = 0, 
  ["umas"] = 0, ["uns"] = 0,

-- english  
  ["a"] = 0, ["about"] = 0, ["above"] = 0, ["after"] = 0, ["again"] = 0, ["against"] = 0,
  ["all"] = 0, ["am"] = 0, ["an"] = 0, ["and"] = 0, ["any"] = 0, ["are"] = 0, ["aren't"] = 0,
  ["as"] = 0, ["at"] = 0, ["be"] = 0, ["because"] = 0, ["been"] = 0, ["before"] = 0,
  ["being"] = 0, ["below"] = 0, ["between"] = 0, ["both"] = 0, ["but"] = 0, ["by"] = 0,
  ["can't"] = 0, ["cannot"] = 0, ["could"] = 0, ["couldn't"] = 0, ["did"] = 0, ["didn't"] = 0,
  ["do"] = 0, ["does"] = 0, ["doesn't"] = 0, ["doing"] = 0, ["don't"] = 0, ["down"] = 0,
  ["during"] = 0, ["each"] = 0, ["few"] = 0, ["for"] = 0, ["from"] = 0, ["further"] = 0,
  ["had"] = 0, ["hadn't"] = 0, ["has"] = 0, ["hasn't"] = 0, ["have"] = 0, ["haven't"] = 0,
  ["having"] = 0, ["he"] = 0, ["he'd"] = 0, ["he'll"] = 0, ["he's"] = 0, ["her"] = 0, ["here"] = 0,
  ["here's"] = 0, ["hers"] = 0, ["herself"] = 0, ["him"] = 0, ["himself"] = 0, ["his"] = 0,
  ["how"] = 0, ["how's"] = 0, ["i"] = 0, ["i'd"] = 0, ["i'll"] = 0, ["i'm"] = 0, ["i've"] = 0,
  ["if"] = 0, ["in"] = 0, ["into"] = 0, ["is"] = 0, ["isn't"] = 0, ["it"] = 0, ["it's"] = 0,
  ["its"] = 0, ["itself"] = 0, ["let's"] = 0, ["me"] = 0, ["more"] = 0, ["most"] = 0, ["mustn't"] = 0,
  ["my"] = 0, ["myself"] = 0, ["no"] = 0, ["nor"] = 0, ["not"] = 0, ["of"] = 0, ["off"] = 0, ["on"] = 0,
  ["once"] = 0, ["only"] = 0, ["or"] = 0, ["other"] = 0, ["ought"] = 0, ["our"] = 0, ["ours"] = 0, 
  ["ourselves"] = 0, ["out"] = 0, ["over"] = 0, ["own"] = 0, ["same"] = 0, ["shan't"] = 0, ["she"] = 0,
  ["she'd"] = 0, ["she'll"] = 0, ["she's"] = 0, ["should"] = 0, ["shouldn't"] = 0, ["so"] = 0,
  ["some"] = 0, ["such"] = 0, ["than"] = 0, ["that"] = 0, ["that's"] = 0, ["the"] = 0, ["their"] = 0,
  ["theirs"] = 0,["them"] = 0, ["themselves"] = 0, ["then"] = 0, ["there"] = 0, ["there's"] = 0,
  ["these"] = 0, ["they"] = 0, ["they'd"] = 0, ["they'll"] = 0, ["they're"] = 0, ["they've"] = 0,
  ["this"] = 0, ["those"] = 0, ["through"] = 0, ["to"] = 0, ["too"] = 0, ["under"] = 0, ["until"] = 0,
  ["up"] = 0, ["very"] = 0, ["was"] = 0, ["wasn't"] = 0, ["we"] = 0, ["we'd"] = 0, ["we'll"] = 0,
  ["we're"] = 0, ["we've"] = 0, ["were"] = 0, ["weren't"] = 0, ["what"] = 0, ["what's"] = 0,
  ["when"] = 0, ["when's"] = 0, ["where"] = 0, ["where's"] = 0, ["which"] = 0, ["while"] = 0,
  ["who"] = 0, ["who's"] = 0, ["whom"] = 0, ["why"] = 0, ["why's"] = 0, ["with"] = 0, ["won't"] = 0,
  ["would"] = 0, ["wouldn't"] = 0, ["you"] = 0, ["you'd"] = 0, ["you'll"] = 0, ["you're"] = 0,
  ["you've"] = 0, ["your"] = 0, ["yours"] = 0, ["yourself"] = 0, ["yourselves"] = 0
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
      if (#w > 1) and (searchEngine.stopWords[w] == nil) and (tonumber(w) == nil) then
        if searchEngine.wordCount[w] == nil then
          searchEngine.wordCount[w] = 0
        end
        searchEngine.wordCount[w] = searchEngine.wordCount[w] + 1
      end
    end
  end
  return 1
end

function searchEngine:indexDocument(uri)
  local dom = DOM.new(uri)
  dom:setVisitFunction(searchEngine.visitDOMNode)
  searchEngine.wordCount = {}
  dom:traverse(0, dom:body()) -- 1 = breadth first; 0 = depth first
  
  self.db:exec("begin transaction;")
  local wc = self:indexWords(dom:getURI(), searchEngine.wordCount)
  self.db:exec("commit transaction;")
  return wc
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
  
  -- convert word string to word id
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
  for docId in self.db:urows("select distinct document from wordcount where word in (" .. wordIdList .. ");") do
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
