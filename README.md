# Web Structured Extraction

## webMining binaries

Download [Windows x86_64 static binaries](https://drive.google.com/file/d/1ponUQXFyFTiuQ05dqfWPlIW0JtAl06va/view?usp=sharing)

### Usage

For offline documents:
```bash
./run.sh file.html
```

For online documents, using remotely controlled browser (Chrome + w3c web driver):
```bash
./webDriver.sh http://host/uri
```

### Example script (basic.lua)

To run the script bellow, open chromedriver in another terminal and:
```bash
./webMining -debug -i basic.lua http://url > output.txt 2> output.debug.txt
```

#### basic.lua
```lua
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
```

## Publications
[Paper published in CIKM'17]( https://www.researchgate.net/profile/Roberto_Velloso/publication/320882865_Extracting_Records_from_the_Web_Using_a_Signal_Processing_Approach/links/5a02ed2caca2720c3263ab7b/Extracting-Records-from-the-Web-Using-a-Signal-Processing-Approach.pdf)

[Paper published in ICWE'19](
https://www.researchgate.net/publication/332666543_Web_Page_Structured_Content_Detection_Using_Supervised_Machine_Learning)

## Building from source
```bash
make
```

### deps (Ubuntu and Windows)
libcurl3-dev, liblua5.3-dev, python3-dev
