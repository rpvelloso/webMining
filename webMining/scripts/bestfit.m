clc;
clear all;
close all;
pkg load optim;

% input size
x=[846 933 278 525 555 401 1153 550 1620 313 1198 1020 271 1834 1494 1667 1289 ...
   1249 725 1556 335 859 924 280 600 210 601 838 418 369 684 322 222 710 831 ...
   884 182 517 1107 417 551 330 661 323 1662 738 614 245 219 1030 595 780 787 ...
   787 4017 3942 3822 4215 4049 3598 4178 4121 2287 3945 3890 3881 545 481 461 ...
   2482 2371 2159 2263 2296 1836 1061 963 1006 969 986 468 2519 2521]';

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
   0.089000 0.105000]';

funcs = {
{@(x, p) p(1)*log2(p(2)*x) ,"O(log(n))"},
{@(x, p) p(1)*x +p(2),"O(n)"},
{@(x, p) p(1)*x.*log2(p(2)*x),"O(nlog(n))"},
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

xx = linspace(min(x), max(x));
bestfit = "";
for i=1:1:length(funcs)
	if maxR2 == fitness{i}{2};
		sd = std(y);
		plot(xx,funcs{i}{1}(xx,fitness{i}{1}).-2*sd,'k:');
		plot(xx,funcs{i}{1}(xx,fitness{i}{1}),'k-');
		plot(xx,funcs{i}{1}(xx,fitness{i}{1}).+2*sd,'k:');
		bestfit = [funcs{i}{2} " R2 = " num2str(maxR2)];
	end
end
ylabel("running time (in seconds)");
xlabel("document size (in # of nodes)");
legend("data", [bestfit], "95%");

return
