clc;
clear all;
close all;
pkg load signal;


% signal def
sig = [
179 180 181 182 183 184 185 186 187 188 189 190 191 192 193 194 190 195 196 197 198 199 200 201 202 203 204 205 206 207 208 209 223 224 225 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 190 191 192 193 194 190 195 196 226 227 228 229 197 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 190 191 192 193 194 190 195 196 230 231 197 232 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 190 191 192 193 194 190 195 196 233 234 235 236 197 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 190 191 192 193 194 190 195 196 197 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 190 191 192 193 194 190 195 196 233 234 235 236 197 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 190 191 192 193 194 190 195 196 233 234 235 236 197 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 237 238 239 184 185 186 187 188 189 190 191 192 193 194 190 195 196 197 232 198 199 200 201 202 203 204 205 206 207 208 209 240 241 242 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 233 234 235 236 197 232 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 243 244 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 243 244 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 240 241 242 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 243 244 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 243 244 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 243 244 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 240 241 242 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 240 241 242 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 233 234 235 236 197 232 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 233 234 235 236 197 232 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 243 244 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 243 244 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 243 244 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 243 244 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 226 227 228 229 197 198 199 200 201 202 203 204 205 206 207 208 209 223 224 225 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 237 238 239 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 240 241 242 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 243 244 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 226 227 228 229 197 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 233 234 235 236 197 232 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 237 238 239 184 185 186 187 188 189 233 234 235 236 197 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 243 244 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 243 244 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 233 234 235 236 197 232 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 243 244 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 243 244 216 217 218 219 220 221 222 179 180 181 182 183 184 185 186 187 188 189 197 232 198 199 200 201 202 203 204 205 206 207 208 209 243 244 216 217 218 219 220 221 222 
];
signal = sig - mean(sig);
signal = signal .* hann(length(signal))';
signal = [signal zeros(1,2*length(signal))];
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
psd = zscore(psd);
psd_dev = std(psd);
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

