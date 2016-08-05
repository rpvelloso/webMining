loadfile("../scripts/dbIndex.lua")()

function usage()
  print("usage:")
  print("./webMining -debug -i index.lua query \'search for something\'")
  print("./webMining -debug -i index.lua index path/to/document.html")
end

if #args > 5 then
  local option = args[5]
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
      print(string.format("docId=%05d, doc=%s,\tscore=%f",docId, searchEngine:getDocumentURI(docId),score))
    end
  else
    usage()
  end

end
