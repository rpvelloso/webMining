loadfile("../scripts/dbIndex.lua")()

-- code to retrieve results
--[=[
local docs = processQuery(db, "computador aqui")

for k,v in pairs(docs) do
  print("docId = ",v[1],", score = ",v[2])
end

do return end
--]=]


-- code to index a document
if #args > 4 then
  indexDocument(args[5])
end
