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

db = initdb()
--ddl(db)