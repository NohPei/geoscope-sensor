
if runLevel > 2
    %baseDir = "E:\Individual_pen\geoscope_data";
    baseDir = 'C:\Users\Adeola\temp\pigdata';
    % pick a day
    dateStr = "2019-05-18";
    % get their filenames - maybe i should just directly process the files
    dataFiles = cell(3,1);
    
    % numbering of sensors is from bottom to top, left to right
    % each column is a cage
    sensorIDs = [159;154;160;155;158;153;156;151;157;152] + (0:3)*10;
    % don't know the spacing between 1+2 <-> 3+4
    penSpace = 1.0;
    penSize = [2.0 2.4];
    penNum = 4;
    penLoc = [0, penSize(2)+penSpace; penSize(1), penSize(2)+penSpace; 0,0; penSize(1), 0]; % bottom left
    sensorLoc = [0.55, 0.65; 0.65, 0.65; 1.35, 0.65; 1.45, 0.65; 0.85, 1.25; 0.95, 1.25; 0.55, 1.75; 0.65, 1.75; 1.35, 1.75; 1.45, 1.75];
    
    penSensors = sensorIDs(:,penNum);
    nSensors = length(penSensors);
    sensorNames = compose("GEOSCOPE_SENSOR_%d", penSensors);
    
    selectedLoc = sensorLoc;
    
    for ii=1:nSensors
        sensorDir = join([baseDir, dateStr, sensorNames(ii),"*.json"],'\');
        fileInfo = dir(sensorDir);
        %
        filenames = cell2mat({fileInfo.name}');
        dataFiles{ii} = filenames;
    end
    
    % [A]collect the data for each sensor, for each json file in the day
    sensorData = cell(3,1);
    for ii=1:nSensors
        dataIn = aggregateSensorData(join([baseDir, dateStr, sensorNames(ii)],'\'), dataFiles{ii}, sensorNames(ii));
        if isempty(dataIn)
            dataIn = [nan, nan];
        end
        sensorData{ii} = dataIn;
    end
    runLevel = runLevel - 1;
end
if runLevel > 1
    % next, find the earliest, latest time where all sensors have data
    timeRanges = zeros(nSensors,2);
    for ii=1:nSensors
        timeRanges(ii,1) = sensorData{ii}(1,1);
        timeRanges(ii,2) = sensorData{ii}(end,1);
    end
    startTime = nanmax(timeRanges(:,1)); % latest starting timestamp
    endTime = nanmin(timeRanges(:,2));% earliest ending timestamp
    
    % ?polate all sensor data to this range
    % 100 windows per second = 100Hz original resolution
    % let's take that down to 1Hz (still 86400 estimates/day)
    %
    polationIdxs = (ceil(startTime/1000):floor(endTime/1000))*1000;
    polatedData = zeros(size(polationIdxs,2), nSensors);
    for ii=1:nSensors
        rawData = sensorData{ii};
        if all(isnan(rawData))
            pd = zeros(size(polationIdxs,2),1);
        else
            nanRows = isnan(rawData(:,1));
            rawData = rawData(~nanRows,:);
            pd = interp1(rawData(:,1), rawData(:,2), polationIdxs, 'linear', 'extrap');
        end
        polatedData(:,ii) = pd;
    end
    runLevel = runLevel - 1;
end
% pretend sensors are the source of the energy and make them radiate gaussian
spread = 1/3; % the std of the gaussian
gridSize = 0.1;
[xs, ys] = meshgrid(0:gridSize:penSize(1),0:gridSize:penSize(2));
baseGaussian = zeros([size(xs),nSensors]);
for ii=1:nSensors
    xPart = ((xs - selectedLoc(ii,1)).^2)/(2*spread^2);
    yPart = ((ys - selectedLoc(ii,2)).^2)/(2*spread^2);
    baseGaussian(:,:,ii) = exp(-xPart-yPart);
end
% to look at these gaussians: surf(xs, ys, baseGaussian(:,:,2)) etc

% add up all the radiations and animate the energy distribution

% gather the data into 1h windows, overlapping by 30 mins
hourlyData = performShortTimeAnalysis(polatedData, @nanmean, 3600, 0.5*3600);
scaleLimit = [min(hourlyData(:)) max(hourlyData(:))];

nFrames = size(hourlyData,1);
F(nFrames) = struct('cdata',[],'colormap',[]);
vw = VideoWriter('heatmap_demo.mp4','MPEG-4');
%vw.Quality = 50;
vw.FrameRate = 1;
open(vw);
f1 = figure;
for ii=1:nFrames
    output = zeros(size(xs));
    coeffs = (hourlyData(ii,:));
    for jj = 1:nSensors
        output = output + coeffs(jj).*baseGaussian(:,:,jj);
    end
    figure(f1);
    hold on
    %surf(xs,ys,output, 'EdgeColor', 'none');
    img = imagesc(0:gridSize:penSize(1),0:gridSize:penSize(1),output);
    set(gca, 'YDir', 'normal');
    caxis(scaleLimit);
    % show the sensor positions as rectangles
    plot(selectedLoc(:,1),selectedLoc(:,2),'r^', 'MarkerSize',10);
    view(0,90);
    axis tight;
    hold off
    F(ii) = getframe;
    writeVideo(vw,F(ii));
end
% save it as an mp4 (wow MATLAB is so cool these days)
close(vw);


% ???
% profit!


% [A]collect the data for one day by:
%%% loading a json file
%%% finding the total energy and time of each packet of data
%%% adding the window timestamp and energy into a matrix
%   for each sensor, for each json file in the day

function [sensorData] = aggregateSensorData(baseDir, fileList, sName)
% assume all the files have 100 blobs of data each
% and each blob has 501 samples, representing 1s of data
windowsPerFile = 100;
samplesPerWindow = 501;
nFiles = size(fileList,1);
%TODO: add columns for window max, window mean, window var
sensorData = nan(nFiles*windowsPerFile, 2);
idx=1;
h=waitbar(0, compose(["Processing sensor ", sName]), 'Interpreter', 'none');
for ii=1:nFiles
    waitbar(ii/nFiles,h);
    fname = join([baseDir string(fileList(ii,:))], '\');
    try
        tmp = jsondecode(fileread(fname));
        % tmp should have windowsPerFile entries
        for jj=1:windowsPerFile
            % convert the data to numbers and center on 0
            winData = eval(tmp(jj).data);
            if (any(winData))
                % clean up those weird negative spikes
                winData = winData - 2048;
                lowThresh = -abs((mean(winData)+3*std(winData)));
                winData(winData<lowThresh) = nan; % remove the negative outliers (>3 std)
                [cleanedData] = fillmissing(winData, 'movmean',3);
            else
                cleanedData = winData;
            end
            winTime = tmp(jj).timestamp;
            sensorData(idx,:) = [winTime, sum(cleanedData.^2)];
            idx = idx+1;
        end
    catch ME
    end
    
end
close(h);

end
