% do some voodoo to smooth the signal
extractedData = arrayfun(@(x)(eval(x.data)), rawSensor, 'UniformOutput', false);
extractedData = cell2mat(extractedData);
% should be 100 rows by 500 cols
% going to use the first 500 cols as-is, then take average of 501 and 1 of
% next row, in order to remove 10Hz jumps
[nRows, nCols] = size(extractedData);
tmp = zeros(numel(extractedData),1);
% there must be a more efficient way but a loop will do for now
for ii=1:nRows-1
    destIdx = 1 + (ii-1)*nCols;
    tmp(destIdx:destIdx+nCols-2) = extractedData(ii,1:nCols-1);
    tmp(destIdx+nCols-1) = (extractedData(ii,nCols)+extractedData(ii+1,1))/2;
end
lastIdx = 1 + (nRows-1)*nCols;
tmp(lastIdx:end) = extractedData(nRows,:);

% now clean up the negative spikes
tmp = tmp-2048; % make the middle of the data range zero
dataStd = std(tmp);
tmp(tmp<-3*dataStd) = nan; % remove the negative outliers (>3 std)
[cleanedData, replaced] = fillmissing(tmp, 'linear');

