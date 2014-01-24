clc;
clear all;
close all;

function filteredAlphabet = filterAlphabet(alphabet, symbolCount, threshold)
  filteredAlphabet = [];
  for i=1:length(alphabet)
    if symbolCount(alphabet(i)) >= threshold
      filteredAlphabet = union(filteredAlphabet,alphabet(i));
    end
  end
  filteredAlphabet = setdiff(filteredAlphabet,[0]);
end

function [pos, mainregion] = searchRegion(tagPathSequence)
  alphabet = [];
  t = 1;
  n = length(tagPathSequence);
  
  for i=1:length(tagPathSequence)
    symbol = tagPathSequence(i);
    if ~ismember(symbol,alphabet)
      alphabet = union(alphabet,symbol);
      symbolCount(symbol)=0;
    end
    symbolCount(symbol) = symbolCount(symbol)+1;
  end
  thresholds = setdiff(unique(symbolCount),[0]);
  
  regionFound = 0;
  while ~regionFound
    t = t + 1;
    if t > length(thresholds)
      break;
    end
    currentAlphabet = filterAlphabet(alphabet,symbolCount,thresholds(t));
    if length(currentAlphabet) < 2
      break;
    end
    currentSymbolCount = symbolCount;
    regionAlphabet = [];
    gapsize = 0;
    for i=1:length(tagPathSequence)
      symbol=tagPathSequence(i);
      if ismember(symbol,currentAlphabet)
        regionAlphabet = union(regionAlphabet,symbol);
        currentSymbolCount(symbol) = currentSymbolCount(symbol) - 1;
        if currentSymbolCount(symbol) == 0
          currentAlphabet = setdiff(currentAlphabet,symbol);
          if length(intersect(currentAlphabet,regionAlphabet)) == 0
            if (length(currentAlphabet) > 1) && (abs((n-2*i+gapsize)/(n-gapsize)) > 0.02)
              regionFound = 1;
              break;
            else
              gapsize = 1;
            end
          end
        end
      else
        if (gapsize) 
          gapsize = gapsize + 1;
        end
      end
    end
  end
  if regionFound
    if i < floor(n/2)
      tagPathSequence = tagPathSequence(i+1:n);
      pos = i;
    else
      tagPathSequence = tagPathSequence(1:i);
      pos = 0;
    end
  else
     pos = -1;
  end
  mainregion = tagPathSequence;
end

x = load('Debug\x');
%output=[1 2 3 4 5 6 4 5 6 7 8 9 10 7 8 9 10 10 10 10 10 10]';

tps=x';


i = 0;
pos = 1;

while i >= 0
  [i, datareg] = searchRegion(tps);
  if i>=0
     pos = pos + i;
     tps = datareg;
  end
end

d_order=2;
mtps=mean(tps);
dd=diff(tps-mtps,d_order);
dd(find(dd > 0))=0;

figure;
hold;
%plot([1:length(x)],x','k.');
plot([1:pos],x'(1:pos),'k.');
plot([pos+length(tps):length(x)],x'(pos+length(tps):length(x)),'k.');
plot([pos:pos+length(tps)-1],tps,'kx');
title('TPS de pagina do site Youtube');
xlabel('posicao da sequencia');
ylabel('codigo tag path');
legend('TPS','','Regiao principal','location','northwest');
legend('boxon');
%plot([pos+d_order:pos+length(tps)-1],dd+mtps,'g');

v=unique(dd+1);
%q=[];
for i=1:length(v)
  f = find(dd <= v(i));
  %f = setdiff(f,q);
  %q = union(q,f);
  m = mean(diff(f));
  l = length(f)+1;
  if (m>=10) && (m<=length(tps)/3)
    r(i,1) = m;
    r(i,2) = l;
    r(i,3) = l*m; %log(l)*log(m);
  else
    r(i,1) = l;
    r(i,2) = m;
    r(i,3) = 0;
  end
end
m=find(r(:,3)==max(r(:,3)));
r=[];
r(1:length(tps))=v(m(1))+mtps;


%plot([pos:pos+length(tps)-1],r,'b');
