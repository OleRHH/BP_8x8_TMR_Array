%s = serialport('/dev/ttyUSB1', 'BaudRate', Bautrate); % Linux RS485
    %s = serialport('/dev/ttyS0', Bautrate);     % Linux com port 
%s = serialport('COM13', 'BaudRate', Bautrate); % Windows 10 com port

Bautrate = 115200; 

if (exist('s') == 0)
    s = serialport('/dev/ttyUSB0', Bautrate);   % RS232 (USBtoRS232 Adapter)
end
if (exist('t') == 1)  
    stop(t)
end
t = timer('ExecutionMode','fixedRate','Period',0.1,'TimerFcn',{@meineFunktion s });

flush(s);
start(t);

while(1)
   
   if s.NumBytesAvailable >= 256
        data = read(s, 128, 'int16');
        sind = data(1:64);
        cosd = data(65:end);

        sind = reshape( sind, [8, 8] )';
        cosd = reshape( cosd, [8, 8] )';

        quiver(cosd, sind), axis square

        xlim([0 9])
        ylim([0 9])

        drawnow
   end
end

function meineFunktion (hObject, eventdata, s)
    %disp(datestr(now,'HH:MM:SS.FFF'))
    write(s, '00000000', 'uint8')
    %flush(s)
    return
end