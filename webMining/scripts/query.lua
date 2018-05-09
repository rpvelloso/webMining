gnuplot = "c:\\Progra~2\\gnuplot\\bin\\gnuplot.exe"
--gnuplot = "/usr/bin/gnuplot"
CRLF = "\n"

term = {}
term["png"] = ".png"
term["postscript"]=".ps"
term["svg"]=".svg"

-- default terminal type (vector graphics)
term["default"]="svg"

--loadfile("../scripts/db.lua")()
loadfile("../scripts/plot.lua")()
loadfile("../scripts/output.lua")()
loadfile("../scripts/testbed.lua")()

driver = webDriver.fireFox
driver:newSession()

function urlencode(str)
   if (str) then
      str = string.gsub (str, "\n", "\r\n")
      str = string.gsub (str, "([^%w ])",
         function (c) return string.format ("%%%02X", string.byte(c)) end)
      str = string.gsub (str, " ", "+")
   end
   return str    
end

sites = {
"https://www.submarino.com.br/busca/___QUERY___",
"https://www.americanas.com.br/busca/___QUERY___",
"https://www.magazineluiza.com.br/busca/___QUERY___/",
"https://search3.pontofrio.com.br/busca?q=___QUERY___",
"https://www.carrefour.com.br/busca/?termo=___QUERY___",
"https://buscas.casasbahia.com.br/?strBusca=___QUERY___",
"https://buscando2.extra.com.br/busca?q=___QUERY___",
"https://www.shoptime.com.br/busca/___QUERY___",
"https://www.fastshop.com.br/web/s/___QUERY___",
"https://www.fnac.com.br/#!Popup_opzSearch=termos--___QUERY___",
"https://www.amazon.com.br/s?k=___QUERY___",
"https://www.walmart.com.br/busca/?ft=___QUERY___",
"http://www.lojahp.com.br/?strBusca=___QUERY___",
"https://www.br.vaio.com/resultadopesquisa/t/8?pag=1&departamento=&buscarpor=___QUERY___&smart=0",
"https://www.audiovideoecia.com.br/consulta.php?palavra=___QUERY___&cod_departamento=",
"https://busca.colombo.com.br/search#?p=Q&ts=ajax&w=dose%20ela&pw=___QUERY___&rt=spelling&isort=score&view=grid",
"https://www.novomundo.com.br/___QUERY___",
"http://busca.lojadomecanico.com.br/?q=___QUERY___",
"https://www.ferramix.com.br/pesquisa/?p=___QUERY___",
"https://www.royalmaquinas.com.br/catalogsearch/result/?q=___QUERY___",
"https://www.netshoes.com.br/busca?nsCat=Natural&q=___QUERY___",
"https://www.zattini.com.br/busca?nsCat=Natural&q=___QUERY___",
"https://www.dafiti.com.br/catalog/?q=___QUERY___&wtqs=1",
"https://esportes.centauro.com.br/search?w=___QUERY___",
"https://shop.modait.com.br/Sistema/buscavazia?ft=___QUERY___",
"https://www.notetec.com.br/catalogsearch/result/?q=___QUERY___",
"http://busca.drogaraia.com.br/search?w=___QUERY___",
"https://www.onofre.com.br/search?N=0&Ntt=___QUERY___",
"https://www.drogariaspacheco.com.br/___QUERY___",
"https://www.drogariasaopaulo.com.br/___QUERY___",
"http://busca.drogasil.com.br/search?w=___QUERY___",
"https://www.comclick.com.br/loja/busca.php?loja=572273&palavra_busca=___QUERY___",
"http://www.balaodainformatica.com.br/placa%20z370?&utmi_p=_&utmi_pc=BuscaFullText&utmi_cp=___QUERY___",
"https://www.kabum.com.br/cgi-local/site/listagem/listagem.cgi?string=___QUERY___&btnG=",
"http://busca.creativecopias.com.br/placa%20z370?q=___QUERY___",
"http://www.estrela10.com.br/pesquisa?t=___QUERY___",
"https://www.chinamix.com.br/buscar?q=___QUERY___",
"https://busca.saraiva.com.br/busca?q=___QUERY___",
"http://www.bibliomedlivraria.com.br/___QUERY___/",
"http://www.mercadodelivros.com.br/___QUERY___/",
"https://www.livrariacultura.com.br/busca?N=0&Ntt=___QUERY___",
"https://www.dilivros.com.br/___QUERY___/",
"https://www.estantevirtual.com.br/busca?utf8=%E2%9C%93&type=q&new=&q=___QUERY___",
"https://busca.martinsfontespaulista.com.br/___QUERY___?q=___QUERY___",
"https://www.kalunga.com.br/busca/___QUERY___/1",
"https://www.globobooks.com.br/busca/___QUERY___",
"https://www.livrariaflorence.com.br/___QUERY___?map=ft",
"https://lista.mercadolivre.com.br/___QUERY___",
"https://www.buscape.com.br/search/___QUERY___",
"https://www.zoom.com.br/search?q=___QUERY___",
"https://pt.aliexpress.com/wholesale?SearchText=___QUERY___",
"http://www.dx.com/s/___QUERY___",
"http://www.kdpneus.com.br/___QUERY___",
"https://www.pneustore.com.br/search/?text=___QUERY___",
"http://www.tireshop.com.br/4/___QUERY___/q",
"https://www.containercultura.com.br/busca/?q=___QUERY___/",
"https://busca.brastemp.com.br/busca?q=___QUERY___",
"http://www.bebestore.com.br/bebestore/busca?q=___QUERY___",
"https://www.amaramar.com.br/Busca/___QUERY___/",
"https://www.tricae.com.br/catalog/?q=___QUERY___"
}

query = "michelin primacy 3 205/55R16" -- args[5]

minPSD = 11
--minCV = 0.30

for i, site in ipairs(sites) do
  local s = string.gsub(site, "___QUERY___", query)
  local i = string.find(s, "://") + 3
  local j = string.find(string.sub(s, i), "/") + i - 2
  print(s)
  driver:go(s)
  processUrl(driver:getCurrentURL(), driver:getPageSource(), "results/", string.sub(s, i, j)..".html")
end 
