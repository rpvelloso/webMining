loadfile("../scripts/dbIndex.lua")()

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

visit = function (dom, node)
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

tf_idf(db, 3)
do return end

wordCount = {}
if #args > 4 then
  local dom = DOM.new(args[5])
  dom:setVisitFunction(visit)
  dom:traverse(0, dom:body()) -- 1 = breadth first; 0 = depth first
  
  indexDocument(db, dom:getURI(), wordCount)
end

for w,c in pairs(wordCount) do
  print(w,"=",c)
end
