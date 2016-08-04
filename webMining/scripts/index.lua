loadfile("../scripts/dbIndex.lua")()

if #args > 4 then
  -- code to index a document
  --searchEngine:indexDocument(args[5])

  -- code to retrieve results
  local docs = searchEngine:processQuery(args[5])
  
  for k,v in pairs(docs) do
    print("docId = ",searchEngine:getDocumentURI(v[1]),", score = ",v[2])
  end

end
