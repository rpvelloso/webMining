minPSD = 9.0
minCV = 0.35
fourier = true

processTestBed = function(dir, generateOutput)
  local t, popen = {}, io.popen
  generateOutput = generateOutput or 1

  for filename in popen('ls -a '..dir..'/*.htm*'):lines() do
    local d, fn, ext = filename:match("(.-)([^\\/]-%.?([^%.\\/]*))$")
    local output = d.."srde/"..fn
    
    print(string.format("Loading DOM tree: %s",filename),CRLF)
    local dom
    local dsre = DSRE.new()
    dsre:setUseFourier(fourier)
    dsre:setAlignmentStrategy(1)
    dsre:setMinPSD(minPSD)
    dsre:setMinCV(minCV)
    
    local start = os.clock()

    if driver ~= nil then
      -- driver:go(filename)
      driver:go('file://c:\\Users\\rvelloso\\workspace' .. filename:sub(6):gsub('/','\\'))
      dom = DOM.new(filename, driver:getPageSource())
    else
      dom = DOM.new(filename)
    end
  
    -- print("Extracting records.")
    dsre:extract(dom)
    print(string.format("elapsed time: %.6f",os.clock() - start),";",dom:size(),CRLF)
    
    if generateOutput == 1 then
      --print("Outputting results.")
      displayResults(dsre,d.."srde/",fn)
      
      --print("Plotting graphs.")
      plotSequences(dsre,"file",output)
    end
    --saveDB(dom,dsre)
    end    
end

processFile = function(filename)
    print(string.format("Loading DOM tree: %s",filename),CRLF)
    local dom
    local dsre = DSRE.new()
    dsre:setAlignmentStrategy(1)
    dsre:setUseFourier(fourier)
    dsre:setMinPSD(minPSD)
    dsre:setMinCV(minCV)
    
    print("Extracting records.")
    local start = os.clock()

    if driver ~= nil then
      --driver:go('file://c:\\Users\\rvelloso\\workspace' .. filename:sub(6):gsub('/','\\'))
      driver:go(filename)
      dom = DOM.new(filename, driver:getPageSource())
    else
      dom = DOM.new(filename)
    end

    dsre:extract(dom)
    print(string.format("elapsed time: %.2f",os.clock() - start),CRLF)
    
    print("Outputting results.")
    displayResults(dsre,"./","output.html")
    
    print("Plotting graphs.")
    plotSequences(dsre,"file","output.html")
    
    dom:printHTML()
    dsre:printTps()
end

processUrl = function(url, html, dir, output)
    print(string.format("Loading DOM tree: %s",url),CRLF)
    local dom = DOM.new(url,html)
    local dsre = DSRE.new()
    dsre:setAlignmentStrategy(1)
    dsre:setUseFourier(fourier)
    dsre:setMinPSD(minPSD)
    dsre:setMinCV(minCV)
    
    print("Extracting records.")
    local start = os.clock()

    dsre:extract(dom)
    print(string.format("elapsed time: %.2f",os.clock() - start),CRLF)
    
    print("Outputting results.")
    displayResults(dsre,dir,output)
    
    print("Plotting graphs.")
    plotSequences(dsre,"file",dir.."/"..output)
    
    --dom:printHTML()
    --dsre:printTps()
end
