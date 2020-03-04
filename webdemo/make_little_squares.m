penSpace = 1.0;
penSize = [2.0 2.4];
penNum = 4;
%penLoc = [0, penSize(2)+penSpace; penSize(1), penSize(2)+penSpace; 0,0; penSize(1), 0]; % bottom left
%baseSensorLoc = [0.55, 0.65; 0.65, 0.65; 1.35, 0.65; 1.45, 0.65; 0.85, 1.25; 0.95, 1.25; 0.55, 1.75; 0.65, 1.75; 1.35, 1.75; 1.45, 1.75];
%sensorLoc = repmat(baseSensorLoc, 4,1) + repelem(penLoc,10,1);
 % numbering of sensors is from bottom to top, left to right
 % each column is a cage
%sensorIDs = [159;154;160;155;158;153;156;151;157;152] + (0:3)*10;
windowSize = 3600; % seconds
windowSlide = windowSize/2;

dataSizes = cellfun(@(x)(size(x,1)), {pen1Data, pen2Data, pen3Data, pen4Data});
lim = min(dataSizes);

perPenData = [nanmean(pen1Data(1:lim,:),2) nanmean(pen2Data(1:lim,:),2) nanmean(pen3Data(1:lim,:), 2) nanmean(pen4Data(1:lim,:),2)];
hourlyData = performShortTimeAnalysis(perPenData, @sum, windowSize, windowSlide);
scaleFactor = min(abs(hourlyData));
hourlyDataNorm = hourlyData./scaleFactor;
allHrs = 0.5:0.5:24;
maxLevel = max(hourlyDataNorm(:));
hourlyDataNorm = (hourlyDataNorm./maxLevel)*100; % percent?


f = figure;
plot(allHrs, hourlyDataNorm);
legend('Pen 1', 'Pen 2', 'Pen 3', 'Pen 4')
xticks(0:24)
xlim([0 24])
xlabel('Hour')
ylabel('Activity level')
title('Pig pen activity during May 18 2019')

dayIdxs = (allHrs >= 6) & (allHrs <=18);
nightIdxs = ~dayIdxs;
f=figure;
makeThePenPlot(mean(hourlyDataNorm(dayIdxs,:)),f);
caxis([0 100])
title('Daytime activity level');
f=figure;
makeThePenPlot(mean(hourlyDataNorm(nightIdxs,:)),f);
caxis([0 100])
title('Nighttime activity level');
% figure(3)
% vw = VideoWriter('heatmap_may18.mp4','MPEG-4');
% vw.FrameRate = 2;
% open(vw);
% % make a movie
% for ii=1:48
%     makeThePenPlot(hourlyDataNorm(ii,:),3);
%     colormap jet
%     caxis([0 1])
%     %title(timeLabel(allHrs(ii)));
%     text(0.4,0.5, timeLabel(allHrs(ii)), 'Fontsize', 14, 'BackgroundColor', [0.5 0.5 0.5]);
% 
%     ff = getframe;
%     writeVideo(vw,ff);
% end
% close(vw);



f = figure;
N = length(allHrs);
idxs = 1:2:N; % skip the half hours
for ii = idxs
    makeThePenPlot(hourlyDataNorm(ii,:),f);
    t = floor(allHrs(ii));
    figName = sprintf('activity%d.png',t);
    saveas(f,figName);
end

function makeThePenPlot(penVals, f)
  shapedVals = reshape(penVals,2,2)';
figure(f);
imagesc([0 1],[0 1],shapedVals)
colorbar;
caxis([0 100])
%colormap(jet(8))
line([0.5 0.5],[1.5 -0.5], 'Color', 'w', 'Linewidth',2)
line([1.5 -0.5], [0.5 0.5], 'Color', 'w', 'Linewidth',2)
text(-0.1,-0.25, 'Pen 1', 'Color', 'w', 'Fontsize', 14, 'BackgroundColor', [0.5 0.5 0.5]);
text(-0.1, 0.75, 'Pen 3', 'Color', 'w', 'Fontsize', 14, 'BackgroundColor', [0.5 0.5 0.5]);
text(0.9,-0.25, 'Pen 2', 'Color', 'w', 'Fontsize', 14, 'BackgroundColor', [0.5 0.5 0.5]);
text(0.9,0.75, 'Pen 4', 'Color', 'w', 'Fontsize', 14, 'BackgroundColor', [0.5 0.5 0.5]);
set(gca,'xtick',[],'ytick',[]);
end

function tl = timeLabel(n)

hr = floor(n);
min = (n-hr)*60;
tl = sprintf('%02d:%02d',hr, min);
end
    



