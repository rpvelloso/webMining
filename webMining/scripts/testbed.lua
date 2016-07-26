processTestBed = function(dir, generateOutput)
  local t, popen = {}, io.popen
  generateOutput = generateOutput or 1

  for filename in popen('ls -a "'..dir..'"/*.htm*'):lines() do
    local d, fn, ext = filename:match("(.-)([^\\/]-%.?([^%.\\/]*))$")
    local output = d.."srde/"..fn
    
    print(string.format("Loading DOM tree: %s",filename),CRLF)
    local dom = DOM.new(filename)
    --local dsre = CVSRE.new()
    local dsre = DSRE.new()
    --dsre:setPeriodEstimator(1)
    
    --print("Extracting records.")
    local start = os.clock()
    dsre:extract(dom)
    print(string.format("elapsed time: %.2f",os.clock() - start),CRLF)
    
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
    local dom = DOM.new(filename)
    local dsre = DSRE.new()
    --local dsre = CVSRE.new()
    
    print("Extracting records.")
    local start = os.clock()
    --dsre:setPeriodEstimator(0)
    dsre:extract(dom)
    print(string.format("elapsed time: %.2f",os.clock() - start),CRLF)
    
    print("Outputting results.")
    displayResults(dsre,"./","output.html")
    
    print("Plotting graphs.")
    plotSequences(dsre,"file","output.html")
    
    dom:printHTML()
    dsre:printTps()
end
