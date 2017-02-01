clc;
clear all;
close all;
pkg load optim;

% input size
x=[846 933 278 525 555 401 1153 550 1620 313 1198 1020 271 1834 1494 1667 1289 ...
   1249 725 1556 335 859 924 280 600 210 601 838 418 369 684 322 222 710 831 ...
   884 182 517 1107 417 551 330 661 323 1662 738 614 245 219 1030 595 780 787 ...
   787 4017 3942 3822 4215 4049 3598 4178 4121 2287 3945 3890 3881 545 481 461 ...
   2482 2371 2159 2263 2296 1836 1061 963 1006 969 986 468 ...
   %2575 ...
   2519 2521]';

% running time
y=[0.039000 0.043000 0.006000 0.014000 0.026000 0.011000 0.018000 0.014000 0.104000 ...
   0.009000 0.045000 0.038000 0.004000 0.068000 0.041000 0.054000 0.044000 0.033000 ...
   0.016000 0.045000 0.009000 0.035000 0.027000 0.011000 0.014000 0.005000 0.010000 ...
   0.027000 0.012000 0.016000 0.018000 0.015000 0.008000 0.026000 0.013000 0.038000 ...
   0.008000 0.016000 0.036000 0.013000 0.027000 0.013000 0.011000 0.025000 0.025000 ...
   0.026000 0.016000 0.003000 0.008000 0.020000 0.027000 0.020000 0.020000 0.021000 ...
   0.191000 0.203000 0.173000 0.181000 0.171000 0.119000 0.158000 0.151000 0.091000 ...
   0.167000 0.151000 0.167000 0.012000 0.014000 0.014000 0.060000 0.051000 0.041000 ...
   0.085000 0.064000 0.060000 0.026000 0.024000 0.019000 0.022000 0.028000 0.006000 ...
   %0.496000 ...
   0.089000 0.105000]';

funcs = {
{@(x, p) p(1)*log2(p(2)*x) ,"O(log(n))"},
{@(x, p) p(1)*x +p(2),"O(n)"},
{@(x, p) p(1)*x.*log2(p(2)*x),"O(nlog(n))"},
% {@(x, p) p(1)*x.^p(2),"O(n^c)"},
{@(x, p) p(1)*x.^2 + p(2),"O(n^2)"},
{@(x, p) p(1)*pow2(x*p(2)),"O(2^n)"}
};

maxR2 = -Inf;
for i=1:1:length(funcs)
	[f, p, cvg, iter, corp, covp, covr, stdresid, Z, r2]=leasqr(x,y,[0.001; 0.001],funcs{i}{1},0.00001,100, ones(size(x)));
	maxR2 = max(maxR2, r2);
	fitness{i} = {p, r2};
	[funcs{i}{2} " R2 = " num2str(r2)]
end

figure;
plot(x,y,'k+'); hold;

xx = linspace(max(x), min(x));
bestfit = "";
for i=1:1:length(funcs)
	if maxR2 == fitness{i}{2};
		plot(xx,funcs{i}{1}(xx,fitness{i}{1}),'k');
		bestfit = [funcs{i}{2} " R2 = " num2str(r2)];
	end
end
ylabel("running time (in seconds)");
xlabel("document size (in # of nodes)");
legend("data", [bestfit]);

return


 [b, bint, r, rint, stats] = regress(time', [ones(size(len')), len']);

[p1, error] = polyfit(len, time, 2);

return

 
[p1, error] = polyfit(size, time, 1);
x1 = linspace(max(size), min(size));
[y1, delta] = polyval(p1, x1, error);
plot(size, time, 'k*'); hold;
plot(x1,y1,'k-');
plot(x1,y1+2*delta,'k:');
plot(x1,y1-2*delta,'k:');
%p2 = polyfit(size, time, 2);
%x2 = linspace(max(size), min(size));
%y2 = polyval(p2, x2);
%plot(x2,y2,'b.');
%p3 = polyfit(size, time, 13);
%x3 = linspace(max(size), min(size));
%y3 = polyval(p3, x3);
%plot(x3,y3,'r.');

ylabel('running time in seconds (full processing)');
xlabel('web page size in # of nodes');
legend('samples', 'linear fit','95% interval');
return
	 
	 
	 





% signal def
sig = [
20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 20 21 22 23 22 22 24 25 23 22 22 23 22 22 26 
];
%sig=sig(9:length(sig));
signal = sig - mean(sig);
%signal = signal .* hann(length(signal))';
signal = [signal zeros(1,length(signal))];
scale = length(signal)/length(sig);
n = length(signal);

% ACF
acf = xcorr(signal,"unbiased");
acf = acf(n:length(acf));
acf = acf - mean(acf);

% DCT
ct = dct(signal);
ct_dev = std(ct);

% PSD
psd = abs(fft(signal)).^2;
psd_dev = std(psd);
psd = zscore(psd);
for i=2:1:length(psd)-1
	if (psd(i-1) < psd(i)) && (psd(i+1) < psd(i))
		peak(i)=1;
	end
end

% plots
figure;

subplot(5,1,1);
plot([0:length(sig)-1],sig,'k.-');

subplot(5,1,2);
axis([0 (n/scale)]);
plot([0:n-1],signal,'k.-');

subplot(5,1,3);
plot([0:n-1],psd,'k.-'); hold on;
plot(find(peak==1)-1,psd(find(peak==1)),'ro');
%plot([0:n-1],ones(1,n)*psd_dev,'r'); hold on;
%plot([0:n-1],ones(1,n)*psd_dev*2,'g'); hold on;
%plot([0:n-1],ones(1,n)*psd_dev*3,'b'); hold on;
%plot([0:n-1],ones(1,n)*psd_dev*4,'y'); hold on;
%plot([0:n-1],ones(1,n)*psd_dev*5,'k'); hold on;
%legend("PSD","std","2*std","3*std");

subplot(5,1,4);
plot([0:n-1],zscore((ct)),'k.-'); hold on;
%plot([0:n-1],ones(1,n)*ct_dev,'r');
%plot([0:n-1],ones(1,n)*ct_dev*2,'g');
%plot([0:n-1],ones(1,n)*ct_dev*3,'b');
%plot([0:n-1],ones(1,n)*ct_dev*4,'y');
%plot([0:n-1],ones(1,n)*ct_dev*5,'k');
%legend("CT","std","2*std","3*std");

subplot(5,1,5);
plot([0:n-1],acf,'k.-');
legend("ACF");

% --------

symbols = unique(sig);
for i=1:length(symbols)
	symbol = symbols(i);
	recpos = unique([1 find(sig==symbol) n]);
	if length(recpos) > 3 
		recsize = diff(recpos);
		m = mean(recsize);
		sd = std(recsize);
		cv = sd/m;
		% recsize std dev; recsize mean; #rec;
		score(symbol,:)=[symbol cv sd m length(recpos)];
	end
end


