initdb = function()
  local db = sqlite3.open("index.db")
  db:exec("pragma foreign_keys = '1';");
  return db
end

ddl = function(db)
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

indexDocument = function(db, uri, wordCount)
  local docId = getDocumentId(db, uri)
  cleanDocWordCount(db, docId)
  for word,count in pairs(wordCount) do
    local wordId = getWordId(db, word)
    insertWordCount(db, docId, wordId, count)
  end
end

tf_idf = function(db, docId)
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
  
  for word,count in pairs(w) do
    print(word,"=",string.format("%.2f",count))
  end
  
  return w
end

db = initdb()
--ddl(db)