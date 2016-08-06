loadfile("../scripts/dbIndex.lua")()

local function usage()
  print("usage:")
  print("./webMining -debug -i index.lua query \'search for something\'")
  print("./webMining -debug -i index.lua index path/to/document.html")
end

local options = {
  ["index"]=0,
  ["query"]=1,
  ["clear"]=2
}

if #args > 5 then
  local option = args[5]
  
  if options[option] == nil then
    usage()
    return
  end
  
  searchEngine:initdb("index.db")
  
  if option == "index" then
    local uri = args[6]
    io.write("indexing "..uri.." ... ")
    local wc = searchEngine:indexDocument(uri)
    io.write(" done: "..wc.." words\n")
  elseif option == "query" then
    local query = args[6]
    local docs = searchEngine:processQuery(query)
    -- display results
    for k,v in pairs(docs) do
      local docId = v[1]
      local score = v[2]
      print(string.format("docId=%05d, uri=%s,\tscore=%f",docId, searchEngine:getDocumentURI(docId),score))
    end
  elseif option == "clear" then
    io.write("reseting database ... ")
    searchEngine:ddl()
    io.write("done\n")
  end
else
  usage()
end

