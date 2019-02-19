function varargout = HumanDet(varargin)
% HUMANDET MATLAB code for HumanDet.fig
%      HUMANDET, by itsesudo modprobe -r iwlwifi mac80211lf, creates a new HUMANDET or raises the existing
%      singleton*.
%
%      H = HUMANDET returns the handle to a new HUMANDET or the handle to
%      the existing singleton*.
%
%      HUMANDET('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in HUMANDET.M with the given input arguments.
%
%      HUMANDET('Property','Value',...) creates a new HUMANDET or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before HumanDet_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to HumanDet_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help HumanDet

% Last Modified by GUIDE v2.5 10-Dec-2018 12:49:49

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @HumanDet_OpeningFcn, ...
                   'gui_OutputFcn',  @HumanDet_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT


% --- Executes just before HumanDet is made visible.
function HumanDet_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to HumanDet (see VARARGIN)

% Choose default command line output for HumanDet
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes HumanDet wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = HumanDet_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global Email;
global Running;
global t;
global Sps ecure;
global SSIP;
global SSConn;
global Logined;
LoginServer = true;
% SSConn = false;
Secure = 0;
Running = false;
Logined = false;
Addr = '127.0.0.1';%'192.168.1.100';
SSIP = '192.168.137.171';%'192.168.1.101';
Port = 31417;
Email = sprintf('ormmroorm@gmail.com\r\n');
Password = sprintf('123456\r\n');
t = tcpip(Addr, Port, 'Timeout', 5);
set(handles.IPEdit, 'String', SSIP);
SSConn = CheckConn(SSIP);
if SSConn
    set(handles.IPEdit,'enable','off');
end
if LoginServer
    [Re, UserName] = Login(t, Email, Password);
    if Re
        Logined = true;
        set(handles.UserName,'String', UserName);
        set(handles.UserName, 'ForegroundColor',[0 0 0]);
    else
        Logined = false;
    end
end

% Get default command line output from handles structure
varargout{1} = handles.output;


% --- If Enable == 'on', executes on mouse press in 5 pixel border.
% --- Otherwise, executes on mouse press in 5 pixel border or over UserName.
function UserName_ButtonDownFcn(hObject, eventdata, handles)
% hObject    handle to UserName (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global t;
global Logined;
global Running;
if (~Logined)
    [Email, Password] = GetLogin();
    if ~isempty(Email)
        [Re, UserName] = Login(t, Email, Password);
        if Re
            set(hObject,'String', UserName);
            set(hObject, 'ForegroundColor',[0 0 0]);
        end
    end
else
    Re = questdlg('Rellay Want to QUIT?','Confirm','Yes','No','Yes');
    if strcmp(Re,'Yes')
        Logined = false;
        Running = false;
        set(hObject,'String', 'Please Login ');
        set(hObject, 'ForegroundColor',[0 0 1]);
    end
end


function [Re, UserName] = Login(t, Email, Password)
global Logined;
Re = false;
UserName = '';
TryAgain = true;
while(TryAgain)
    TryAgain = false;
    try
        if strcmp(t.Status, 'closed')
            fopen(t);
        end
        fwrite(t,'  .M5');
        fwrite(t,Email);
        fwrite(t,Password);
        receive = fread(t,4);
        receive(receive == 0) = [];
        if (strcmp(char(receive'), '.U') || strcmp(char(receive'), '.P'))
            h=warndlg('Wrong User Name or Password','Login failed','modal');
            uiwait(h);
            [Email, Password] = GetLogin();
            if ~isempty(Email)
                TryAgain = true;
            end
        end
        if (strcmp(char(receive'), '.Y'))
            receive = fgets(t);
            i = 1 : 2 : length(receive);
            receive(i) = [];
            UserName = receive;
            Re = true;
            Logined = true;
        end
    catch ME
        switch ME.identifier
            case 'instrument:fopen:opfailed'
                h=warndlg('Failed to connect to Server','Connection Failed','modal');
                return; 
            otherwise
                rethrow(ME);
        end
    end
end

function [Email, Password] = GetLogin()
Re = inputdlg({'UserName','Password'},'Login', 1, {'ormmroorm@gmail.com';'123456'});
try
    Email = sprintf([Re{1} '\r\n']);
    Password = sprintf([Re{2} '\r\n']);
catch
    Email = '';
    Password = '';
end


% --- Executes when user attempts to close figure1.
function figure1_CloseRequestFcn(hObject, eventdata, handles)
% hObject    handle to figure1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global t;
global Running;
global Secure;
if Secure ~= 0
    Secure = 0;
    SendAct('U');
end
if (strcmp(t.Status, 'open'))
    fclose(t);
end
if Running
    Running = false;
end

% Hint: delete(hObject) closes the figure
delete(hObject);


% --- Executes on button press in Button_Start.
function Button_Start_Callback(hObject, eventdata, handles)
% hObject    handle to Button_Start (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global Running;
global Htemp;
global Secure;
if (~strcmp(get(hObject, 'String'), 'Start'))
    Running = false;
    set(hObject, 'String', 'Start');
    return;
end
% Parameters
% communication file
    com_file = '/home/jennygroup/Matlab/com';
% Hampel Identifier
    hampel_k = 200;
    hampel_nsigma = 1.3;
% Figure's x axis
    max_plot_time = 8;
% Data processing
    adjust_process = 3; %0: raw data; 1:hampel indentifier; 2: filter
    cutoff = 20;
    refresh_time = 0.5;   
% Time out threshold: s
    time_over = 5;
% antenna 
    antenna = [1 2 3];
% for walk detection
    slen = max_plot_time / 2; %s,for walk detection 
    Threshold = 2;
    no_human_flag = 1;
    slience_time = 5; %s, when activity detected, slience for 5s
    no_act_time = 60; %s, when no activity perform for 1min, turn off the light
% intialize
% Error if file has already started
in = fopen(com_file,'r');
file_name = fgetl(in);
fclose(in);
state = file_name(end);
if state == '1'
    h=warndlg('Please Click Start of this then the program','Start Failed','modal');
    uiwait(h);
    return;
end
Running = true;
set(hObject,'String','Stop');
% axes(handles.figure);
H = handles.FigurePannel;
if ishandle(Htemp)
    delete(Htemp);
end
Htemp = axes('parent',H);
title('Ready');
%Secure = 0; % unknown
% title('Ready');
while Running
    state = '0';
    while state ~= '1'
        if ~Running
            break;
        end
        in = fopen(com_file,'r');
        file_name = fgetl(in);
        fclose(in);
        state = file_name(end);
        pause(refresh_time);
    end
    if state ~= '1' % Break from running turn to false.
        break;
    end
    file_name(end - 1 : end) = [];
    file_name(1 : 7) = [];
    duration = refresh_time;        % refresh time
    plot_start_time = 1;            % used in plot()
    total_time = 0;                 % total time
    cursor_series = ones(1,6000);
    mag = zeros(0,3);
    total_len = 0;
    % File cursor
    % cursor_last = 0;
    cursor_current = 1;
    % Time Out
    time_out = 0;
    time_act = 1;
    Secure = 2;
    while(time_out < time_over)
        if ~Running
            break;
        end
        total_time = total_time + duration;
        cursor_last = cursor_current;
        tic
    % 3.1 Copy the file and Read the file for the CSI raw data
        % copyfile(file_name,'temp.dat','f');
        % Do not copy the file
        [CSI_trace,cursor_current] = read_bf_file_online(file_name, cursor_last);
        if cursor_last == cursor_current
            duration = toc;
            if (duration <= refresh_time)
                pause(refresh_time-duration); 
                duration = refresh_time;
            end
            time_out = time_out + duration;
            continue;
        else
            time_out = 0;
        end
        len_t = size(CSI_trace, 1);
        total_len = total_len + len_t;
        cursor_series(ceil(total_time)) = total_len;
        raw_data = zeros(len_t, 3);
        for i = 1:len_t
            csi_entry = CSI_trace{i}; %for every packet
            csi = get_scaled_csi(csi_entry); % csi(Ntx, Nrx, Channel) 2 * 3 * 30
            csi = squeeze(csi(1, :, 1)); 
            raw_data(i, :) = csi;
        end
        mag = cat(1,mag,abs(raw_data));
        if size(mag,1) < 50
            continue;
        end
    % 3.2 Data Processing
        % 3.2.1 Change the mag size if the total time is lager than 25s
        if total_time > max_plot_time + 2 * refresh_time
            change_plot_start_time = ceil(total_time - max_plot_time);
            while change_plot_start_time > plot_start_time && cursor_series(change_plot_start_time) == 1
                change_plot_start_time = change_plot_start_time - 1;
            end
            mag(1:cursor_series(change_plot_start_time)-cursor_series(plot_start_time),:) = [];
            plot_start_time = change_plot_start_time;
        end
        len = size(mag,1);
        time = total_time - plot_start_time;
%         dataout = mag;
        sample_rate = len / time;
        % 3.2.2 Hampel Identifier -- Outlier Removal just the first subcarrier
        if adjust_process >= 1
            outliered = zeros(len,3);
            for ham_i = 1:3
                outliered(:,ham_i) = hampel(mag(:,ham_i)',hampel_k,hampel_nsigma);
            end
        end
        % 3.2.3 Butterworth low-pass filter
        if adjust_process >= 2
            d = fdesign.bandpass('N,F3dB1,F3dB2',8, 0.3, 2, 867);
            hd = design(d, 'butter');
            after_filter = zeros(len,3);
            for i = 1:3
                after_filter(:,i) = filter(hd, outliered(:,i));
            end
%             dataout = after_filter;
            
        end
        
    % 3.3 Draw figure
        if total_time < max_plot_time + 2101 * refresh_time
            inveral = total_time /len;
            t = 0:inveral:(total_time - inveral);
        else
            inveral = (total_time - plot_start_time)/len;
            t = plot_start_time:inveral:(total_time - inveral);
        end
        if adjust_process >= 2 && t(end) > plot_start_time
            data_out = after_filter(t - total_time + slen > 0, :);
            if total_time - time_act > slience_time && max(max(data_out)) > Threshold
                disp('activity Detected!!!');
                if Secure ~= 1
                    Secure = 1;
                    disp('State change');
                    SendAct('Y');
                    SSAct('On');
                end
                time_act = total_time;
                no_human_flag = 0;
            end
            if no_human_flag == 0 && total_time - time_act > no_act_time
                no_human_flag = 1;
                disp(['No activity for ' num2str(no_act_time) 's !!!']);
                if Secure ~= 2
                    disp('State change');
                    Secure = 2;
                    SendAct('N');
                    SSAct('Off');
                end
            end
        end
        antenna_len = length(antenna);
        hs = cell(1,antenna_len);
        for ii = 1 : antenna_len
            
            hs{ii} = subplot(ceil(antenna_len / 2),2,ii,'Parent', H);
            plot(t,mag(:,ii));
            if adjust_process >= 1
                hold on
                plot(t,outliered(:,ii));
                if adjust_process >= 2
                    plot(t,after_filter(:,ii));
                end
                hold off
            end
            axis tight
            if ii == antenna_len
                title(['antenna: ' num2str(ii) ' file:' strrep(file_name,'\','\\')]);
            else
                title(['antenna: ' num2str(ii)]);
            end
            xlabel('time/s');
            ylabel('amplitude/dB');
        end
        drawnow;
        duration = toc;
        if (duration <= refresh_time)
           pause(refresh_time-duration); 
           duration = refresh_time;
        end
    end
    if exist('antenna_len','var')
        for ii = 1 : antenna_len
            if ishandle(hs{ii})
                delete(hs{ii});
            end
        end
    end
    if ishandle(Htemp)
        delete(Htemp);
    end
    Htemp = axes('parent',H);
    title('Waiting for start');
end
% clf(Handles.figure);
if exist('antenna_len','var')
    for ii = 1 : antenna_len
        if ishandle(hs{ii})
            delete(hs{ii});
        end
    end
end
if ishandle(Htemp)
    delete(Htemp);
end
Htemp = axes('parent',H);
title('Waiting for start');


function SendAct(State)
global t;
global Logined;
if Logined
    fwrite(t,'  .M6');
    temp = sprintf('%s\r\n',State);
    fwrite(t,temp);
    receive = fread(t,4);
    receive(receive == 0) = [];
    receive = receive';
    if (~strcmp(char(receive), '.Y'))
        error('ErrorInSending');
    end
end


function Re = CheckConn(SSIP)
Dots = strfind(SSIP,'.');
flag = true;
if isempty(Dots)
    flag = false;
else
    if length(Dots) ~= 3
        flag = false;
    else
        Dots = [0 Dots length(SSIP)+1];
        for i = 1 : 4
            temp = str2double(SSIP(Dots(i)+1 : Dots(i+1)-1));
            if temp < 0 || temp > 255
                flag = false;
            end
        end
    end
    if flag
        import matlab.net.*
        import matlab.net.http.*
        r = RequestMessage;
        uri = URI(['http://' SSIP]);
        try
            resp = r.send(uri);
            flag = strcmp(resp.StatusCode,'OK');
        catch ME
            switch ME.identifier
                case 'MATLAB:webservices:Timeout'
                    flag = false;
                otherwise
                    rethrow(ME);
            end
            flag = false;
        end
        
    end
    if ~flag
        h=warndlg('Failed to connect to SmartSwitch','Connection Failed','modal');
    end
end
Re = flag;

function Re = SSAct(Act)
import matlab.net.*
import matlab.net.http.*
global SSIP;
global SSConn;
r = RequestMessage;
if ~SSConn
    Re = false;
else
%     flag = true;
    if strcmp(Act,'On')
        uri = URI(['http://' SSIP '/LightOn']);
        resp = r.send(uri);
        flag = strcmp(resp.StatusCode,'OK');
    else
        uri = URI(['http://' SSIP '/LightOff']);
        resp = r.send(uri);
        flag = strcmp(resp.StatusCode,'OK');
    end
    Re = flag;
end



function IPEdit_Callback(hObject, eventdata, handles)
% hObject    handle to IPEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global SSIP;
global SSConn;
SSIP = get(hObject,'String');
SSConn = CheckConn(SSIP);
% Hints: get(hObject,'String') returns contents of IPEdit as text
%        str2double(get(hObject,'String')) returns contents of IPEdit as a double


% --- Executes during object creation, after setting all properties.
function IPEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to IPEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in ConnButton.
function ConnButton_Callback(hObject, eventdata, handles)
% hObject    handle to ConnButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global SSIP;
global SSConn;
SSIP = get(handles.IPEdit,'String');
SSConn = CheckConn(SSIP);
if SSConn
    set(handles.IPEdit,'enable','off');
end