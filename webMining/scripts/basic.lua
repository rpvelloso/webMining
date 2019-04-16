CRLF = '\n'
extractURL = function(url, html, minPSD, minCV) -- loads and parses HTML into a DOM tree 
	local dom = DOM.new(url, html)

	-- instantiates an extractor
	local dsre = DSRE.new() 

	-- sets z-score and CV thresholds
	dsre:setMinPSD(minPSD)
	dsre:setMinCV(minCV)

	-- extract records
	dsre:extract(dom)

	local regions = dsre:regionCount()

	-- iterates over regions
	for i=1,regions do
		local dr = dsre:getDataRegion(i-1)
		local rows = dr:recordCount()
		local cols = dr:recordSize()
		local content = ''
		if dr:isContent() then
			content = 'CONTENT DETECTED'
		end
		print('Region #', i, content, ': ')

	-- iterates over current region's
	-- rows and columns
		for r=1,rows do  
			local record = dr:getRecord(r-1)
			for c=1,cols do
	-- output field value
				if (record[c] ~= nil) then
					io.write(record[c]:toString())
				end
				io.write(';')
			end
			io.write(CRLF)
		end
	end
end

local driver = webDriver.chrome

if #args > 4 then
	local url = args[5]
	driver:newSession()
	driver:go(url)
	local html = driver:getPageSource()
	extractURL(url, html, 9.0, 0.30)
end