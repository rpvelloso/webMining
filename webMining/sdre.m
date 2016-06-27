clc;
clear all;
close all;
pkg load signal;


function [a, b] = linearRegression(y)
   n = length(y);
   x=(1:n);
   xy = x.*y;
   x2=x.^2;
   sx = sum(x);
   sy=sum(y);
   sxy=sum(xy);
   sx2=sum(x2);
   delta=n*sx2-sx^2;
   
   a=(n*sxy-sy*sx)/delta;
	b=(sx2*sy-sx*sxy)/delta;
end

function t = contour(s)
	t = s;
	n = length(s);
	height = s(1);
	for i=1:1:n
		if s(i) > height
			height = s(i);
		end
		t(i) = height;
	end
end

function d = firstDiff(s)
	n = length(s);
	d = s;
	d(2:n) = diff(s);
	d(1) = 1;
end

function r = regions(c)
	count = 0;
	n = length(c);
	d = firstDiff(c);
	
	start = 1;
	finish = 1;
	for i=1:1:n
		if d(i) != 0
			if (finish - start + 1) > 3
				count = count + 1;
				r{count} = [start, finish];
			end
			start = i + 1;
			finish = i + 1;
		else
			finish = i;
		end
	end
	if start != finish
		count = count + 1;
		r{count} = [start, finish];
	end
end

function mr = mergeRegions(r, s)
	count = 1;
	n = length(r);
	mr{1} = r{1};
	for i=2:1:n
		palpha = unique(s(mr{count}(1):mr{count}(2)));
		alpha = unique(s(r{i}(1):r{i}(2)));
		if length(intersect(alpha, palpha)) > 0
			mr{count}(2) = r{i}(2);
		else
			count = count + 1;
			mr{count} = r{i};
		end
	end
	
	n = length(s);
	for i=1:1:length(mr)
		alpha = unique(s(mr{i}(1):mr{i}(2)));
		%while (mr{i}(1) > 1) && (s(mr{i}(1)-1) <= max(alpha)) && (s(mr{i}(1)-1) >= min(alpha))
		while (mr{i}(1) > 1) && (length(intersect(s(mr{i}(1)-1),alpha)) > 0)
			mr{i}(1) = mr{i}(1) - 1;
		end
		
		while (mr{i}(2) < n) && (length(intersect(s(mr{i}(2)+1),alpha)) > 0)
			mr{i}(2) = mr{i}(2) + 1;
		end
	end
end

function sr = detectStructure(r,s)
	count = 0;
	n = length(r);
	for i=1:1:n
		[a,b] = linearRegression(s(r{i}(1):r{i}(2)));
		r{i}(3:4) = [a, b];
		if abs(a) < 0.17453 % 10 degrees
			count = count + 1;
			sr{count} = r{i};
		end
	end
end

function t = transform(sig)
	size = length(sig);
	%lowpass(1:size) = 1/size;
	%lowpass(ceil(size/8):size)=0;
	
	%sig = real(ifft(fft(sig).*lowpass));
	
	%sig = sig .* welchwin(length(sig),"periodic")';
	t=abs(fft(sig)).^2;
end

function rec = findRecords(reg, a, b)
	n = length(reg);
	t = transform(reg - mean(reg));
	xc = xcorr(reg - mean(reg),"biased")(n:(n*2)-1);
	i = 2; % skip begining of xcorr func
	while (xc(i) <= xc(i-1) && i <= length(xc))
		i = i+1;
	end
	[peakCor,pos] = sort(xc(i:length(xc)),"descend");
	pos = pos(1:min(15,length(pos)))+i-1;
	peaksFreq = t(round(n./pos));
	maxPeak = find(peaksFreq==max(peaksFreq));
	period = pos(maxPeak(1));
	
	figure;
	subplot(3,1,1);
	plot(1:n,reg,'k.-'); hold on;
	plot(1:n,(a.*[1:n]) + b,'k--');
	text(n,(a.*n) + b,[num2str(a*180/pi,"%3.2f") '\circ']);
	title("a) região, regressão linear e divisão de registros");

	v = score(reg, period);
	if length(v) > 1
		plot(v,reg(v),'ks');
		legend("Região", "Ângulo", "Registros");
		for i=1:length(v)-1
			rec{i} = reg(v(i):v(i+1)-1);
		end
		rec{length(v)} = reg(v(length(v)):length(reg));
	else
		legend("Região","Ângulo");
	end
	
	subplot(3,1,2);
	plot(xc,'k.-'); hold on;
	plot(pos,xc(pos),'ks');
	legend("XCorr","Picos");
	title("b) Picos de autocorrelação");

	subplot(3,1,3);
	plot(1:n,t,'k.-'); hold on;
	fftPos = round(n./pos);
	maxPos = fftPos(find(t(fftPos) == max(t(fftPos)))(1));
	plot(fftPos,t(fftPos),'ks');
	text(maxPos,t(maxPos),['max: freq=' num2str(maxPos,"%d") ', período=' num2str(round(n/maxPos),"%d")]);
	legend("FFT","XCorr");
	title("c) FFT, picos de autocorrelação e estimativa de período e quantidade de registros");
end

function rec = findRecords2(reg, a, b)
	n = length(reg);
	nDiv2 = round(n/2);
	tt = dct(reg - mean(reg));
	for i=1:length(tt)-3
		%if mod(i,2)==1
		%	t(i) = abs(tt(2*(i-1)+1)) + abs(tt(2*(i-1)+3));
		%else
		%	t(i) = abs(tt(2*(i-1))) + abs(tt(2*(i-1)+2));
		%end
		t(i) = abs(tt(i)) + abs(tt(i+2));
	end
	
	freq = find(t == max(t))(1)/2;
	period = round(n/freq);
	
	printf("f: %d, p:%d\n",freq,period);
	
	figure;
	subplot(2,1,1);
	plot(1:n,reg,'k.-'); hold on;
	plot(1:n,(a.*[1:n]) + b,'k--');
	text(n,(a.*n) + b,[num2str(a*180/pi,"%3.2f") '\circ']);
	title("a) região, regressão linear e divisão de registros");

	v = score(reg, period);
	if length(v) > 1
		plot(v,reg(v),'ks');
		legend("Região", "Ângulo", "Registros");
		for i=1:length(v)-1
			rec{i} = reg(v(i):v(i+1)-1);
		end
		rec{length(v)} = reg(v(length(v)):length(reg));
	else
		legend("Região","Ângulo");
	end
	
	subplot(2,1,2);
	plot(t,'k.-'); hold on;
	plot(2*freq,t(2*freq),'ks');
	legend("DCT","Pico");
	title("b) DCT");
end

function v = score(region, period)
	n = length(region);
	avg = mean(region);
	region = region - avg;
	candidates = sort(unique(region(find(region<0))));
	estFreq = length(region) / period;
	maxScore = -Inf;
	v = 0;
	
	while (length(candidates) > 0)
		value = candidates(1);
		candidates = setdiff(candidates, value);
		
		recpos = find(region == value);
		reccount = length(recpos);
		%avgSize = avg(diff(recpos));
		avgSize = sum(abs(diff(recpos)-period));
		
		coverage = (recpos(reccount) - recpos(1)) / n;
		freqRatio = min(reccount, estFreq) / max(reccount, estFreq);
		%sizeRatio = min(avgSize, period) / max(avgSize, period);
		sizeRatio = 1 - (min(avgSize, n) / max(avgSize, n));
		scr = (coverage + freqRatio + sizeRatio) / 3;
		printf("value=%d, cov=%.2f, #=%.2f, size=%.2f, s=%.4f - %.2f\n",
			round(value+avg),coverage,freqRatio,sizeRatio,scr,period);
			
		if scr > maxScore
			maxScore = scr;
			v = recpos;
			haltScore = 0.75;
			if ((coverage > haltScore) && (freqRatio > haltScore) && (sizeRatio > haltScore))
				printf("\n");
				return;
			end
		end
	end
	printf("\n");
end

function drawPlots(s, c, r)
	d = firstDiff(c);
	figure; % signal only
	plot(s,'k--');
	legend("tps");
	
	figure; % signal and contour
	subplot(1,2,1);
	title("Contorno da sequÃªncia");
	plot(s,'k--'); hold;
	plot(c, 'k.');
	legend("tps", "contorno");
	%plot(d.*mean(c), 'ok');
	
	subplot(1,2,2);
	%figure; % signal and 1st diff of contour
	plot(s,'k--'); hold;
	plot((d!=0) .* c,'k.');
	legend("tps", "derivada do contorno");
	
	figure; % structured regions
	plot(s,'.-'); hold;
	for i=1:1:length(r)
		interval = r{i}(1):r{i}(2);
		reg = s(r{i}(1):r{i}(2));
		plot(interval,reg,'.r');
	end
end

function sr = segment(s)
	c = contour(s);
	r = regions(c);
	mr = mergeRegions(r,s);
	sr = detectStructure(mr,s);
	drawPlots(s, c, sr);
	
	for i=1:1:length(sr)
		reg = s(sr{i}(1):sr{i}(2));
		a = sr{i}(3);
		b = sr{i}(4);
		rec = findRecords2(reg,a,b);
		if length(rec)>1
			printf("Found %d records:\n",length(rec));
		end
		
		for i=1:length(rec)
			for j=1:length(rec{i})
				printf("%d;",rec{i}(j));
			end
			printf("\n");
		end
	end
end

signal = [2,3,4,5,6,7,7,7,8,9,7,8,9,7,10,11,12,13,14,15,16,8,17,8,18,8,19,8,20,8,21,8,22,8,23,8,17,8,24,8,25,8,24,8,24,26,27,28,29,30,31,32,28,29,27,33,34,7,35,36,8,9,7,35,36,37,38,10,11,39,40,41,42,43,44,45,46,45,44,45,44,45,44,45,44,45,47,48,49,48,49,48,49,48,49,48,49,48,49,48,49,48,49,48,49,48,49,48,49,48,49,48,49,48,49,48,49,48,49,48,49,48,49,48,49,48,49,48,49,48,49,48,49,48,49,48,49,48,49,48,49,45,44,50,51,11,52,53,11,54,55,56,57,10,55,58,59,11,60,61,62,10,11,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,77,78,79,77,80,81,77,82,83,77,83,84,83,85,86,77,87,82,83,77,83,84,83,77,87,82,83,77,83,84,83,85,86,77,87,82,83,77,83,84,83,85,86,77,87,82,83,77,83,84,83,85,86,77,87,82,83,77,83,84,83,85,86,77,87,82,83,77,83,84,83,85,86,77,87,82,83,77,83,84,83,85,86,77,87,82,83,77,83,84,83,77,87,82,83,77,83,84,83,85,86,77,87,82,83,77,83,84,83,85,86,77,87,82,83,77,83,84,83,85,86,77,87,82,83,77,83,84,83,85,86,77,87,82,83,77,83,84,83,77,87,82,83,77,83,84,83,85,86,77,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,108,113,108,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,108,113,108,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,108,113,108,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,108,113,108,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,108,113,108,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,108,113,108,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,108,113,108,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,108,113,108,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,108,113,108,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,108,113,108,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,108,113,108,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,108,113,108,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,108,113,108,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,108,113,108,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,108,113,108,75,76,77,80,81,80,81,80,81,80,81,80,81,80,81,80,81,80,81,77,80,81,77,114,115,116,117,118,119,120,121,122,64,10,11,63,64,123,124,125,126,127,128,129,130,131,132,133,134,135,7,7,136,135,7,135,8,9,134,135,7,7,136,135,7,135,8,9,134,135,7,7,136,135,7,135,8,9,134,135,7,7,136,135,7,135,8,9,134,135,7,7,136,135,7,135,8,9,134,135,7,7,136,135,7,135,8,9,134,135,7,7,136,135,7,135,8,9,134,135,7,7,136,135,7,135,8,9,134,135,7,7,136,135,7,135,8,9,134,135,7,7,136,135,7,135,8,9,130,137,138,139,140,141,137,138,139,140,141,137,138,139,140,141,137,138,139,140,141,137,138,139,140,141,137,138,139,140,141,137,138,139,140,141,137,138,139,140,141,137,138,139,140,141,137,138,139,140,141,142,143,144,7,145,146,145,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,163,164,165,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,186,189,190,179,180,181,182,183,184,185,186,191,188,186,189,190,145,145,147,158,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,207,218,219,220,221,222,223,224,225,226,227,207,208,209,210,211,212,213,214,215,216,217,207,218,219,220,221,222,223,224,225,226,227,207,208,209,210,211,212,213,214,215,216,217,145,147,158,228,229,230,231,232,233,234,232,235,236,232,233,234,145,145,237,238,239,240,241,242,240,241,242,240,241,242,240,241,242,243,244,245,240,241,242,240,241,242,240,241,242,240,241,242,240,241,242,240,241,242,243,244,246,247,248,249,247,248,249,247,248,249,247,248,249,246,247,248,249,246,247,250,247];
%signal=[1,2,3,4,5,6,7,8,9,10,11,10,11,10,11,10,11,10,11,12,13,14,15,16,5,6,7,17,17,18,19,20,21,22,23,24,19,25,26,27,28,29,30,31,32,33,34,35,36,37,37,34,35,36,37,37,34,35,36,37,37,34,35,36,37,37,34,35,36,37,37,34,35,36,37,37,34,35,36,37,37,34,35,36,37,37,34,35,36,37,37,38,39,40,38,38,38,38,41,42,43,44];

p = segment(signal);
