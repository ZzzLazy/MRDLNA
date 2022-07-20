//
//  MRDLNA.m
//  MRDLNA
//
//  Created by MccRee on 2018/5/4.
//

#import "MRDLNA.h"
#import "StopAction.h"

NSString *const kMRDLNAGetTransportInfoResponseNotification = @"kMRDLNAGetTransportInfoResponseNotification";
NSString *const kMRDLNASetAVTransportURINotification = @"kMRDLNASetAVTransportURINotification";
NSString *const kMRDLNAPlayResponseNotification = @"kMRDLNAPlayResponseNotification";
NSString *const kMRDLNAPauseResponseNotification = @"kMRDLNAPauseResponseNotification";
NSString *const kMRDLNAStopResponseNotification = @"kMRDLNAStopResponseNotification";
NSString *const kMRDLNASeekResponseNotification = @"kMRDLNASeekResponseNotification";
NSString *const kMRDLNAPreviousResponseNotification = @"kMRDLNAPreviousResponseNotification";
NSString *const kMRDLNANextResponseNotification = @"kMRDLNANextResponseNotification";
NSString *const kMRDLNASetVolumeResponseNotification = @"kMRDLNASetVolumeResponseNotification";
NSString *const kMRDLNAGetVolumeResponseNotification = @"kMRDLNAGetVolumeResponseNotification";
NSString *const kMRDLNASetNextAVTransportURIResponseNotification = @"kMRDLNASetNextAVTransportURIResponseNotification";
NSString *const kMRDLNAGetPositionInfoResponseNotification = @"kMRDLNAGetPositionInfoResponseNotification";
NSString *const kMRDLNAUndefinedResponseNotification = @"kMRDLNAUndefinedResponseNotification";

@interface MRDLNA()<CLUPnPServerDelegate, CLUPnPResponseDelegate>

@property(nonatomic,strong) CLUPnPServer *upd;              //MDS服务器
@property(nonatomic,strong) NSMutableArray *dataArray;

@property(nonatomic,copy) NSString *volume;
@property(nonatomic,assign) NSInteger seekTime;
@property(nonatomic,assign) BOOL isPlaying;

@end

@implementation MRDLNA

+ (MRDLNA *)sharedMRDLNAManager{
    static MRDLNA *instance = nil;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        instance = [[self alloc] init];
    });
    return instance;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        self.upd = [CLUPnPServer shareServer];
        self.upd.searchTime = 5;
        self.upd.delegate = self;
        self.dataArray = [NSMutableArray array];
    }
    return self;
}

/**
 ** DLNA投屏
 */
- (void)startDLNA{
    [self initCLUPnPRendererAndDlnaPlay];
}
/**
 ** DLNA投屏
 ** 【流程: 停止 ->设置代理 ->设置Url -> 播放】
 */
- (void)startDLNAAfterStop{
    StopAction *action = [[StopAction alloc]initWithDevice:self.device Success:^{
        [self initCLUPnPRendererAndDlnaPlay];
        
    } failure:^{
        [self initCLUPnPRendererAndDlnaPlay];
    }];
    [action executeAction];
}
/**
 初始化CLUPnPRenderer
 */
-(void)initCLUPnPRendererAndDlnaPlay{
    self.render = [[CLUPnPRenderer alloc] initWithModel:self.device];
    self.render.delegate = self;
    [self.render setAVTransportURL:self.playUrl];
}
/**
 退出DLNA
 */
- (void)endDLNA{
    [self.render stop];
}

/**
 播放
 */
- (void)dlnaPlay{
    [self.render play];
}


/**
 暂停
 */
- (void)dlnaPause{
    [self.render pause];
}

/**
 搜设备
 */
- (void)startSearch{
    [self.upd start];
}


/**
 设置音量
 */
- (void)volumeChanged:(NSString *)volume{
    self.volume = volume;
    [self.render setVolumeWith:volume];
}


/**
 播放进度条
 */
- (void)seekChanged:(NSInteger)seek{
    self.seekTime = seek;
    NSString *seekStr = [self timeFormatted:seek];
    [self.render seekToTarget:seekStr Unit:unitREL_TIME];
}


/**
 播放进度单位转换成string
 */
- (NSString *)timeFormatted:(NSInteger)totalSeconds
{
    NSInteger seconds = totalSeconds % 60;
    NSInteger minutes = (totalSeconds / 60) % 60;
    NSInteger hours = totalSeconds / 3600;
    return [NSString stringWithFormat:@"%02ld:%02ld:%02ld",(long)hours, (long)minutes, (long)seconds];
}

/**
 播放切集
 */
- (void)playTheURL:(NSString *)url{
    self.playUrl = url;
    [self.render setAVTransportURL:url];
}

#pragma mark -- 搜索协议CLUPnPDeviceDelegate回调
- (void)upnpSearchChangeWithResults:(NSArray<CLUPnPDevice *> *)devices{
    NSMutableArray *deviceMarr = [NSMutableArray array];
    for (CLUPnPDevice *device in devices) {
        // 只返回匹配到视频播放的设备
        if ([device.uuid containsString:serviceType_AVTransport]) {
            [deviceMarr addObject:device];
        }
    }
    if ([self.delegate respondsToSelector:@selector(searchDLNAResult:)]) {
        [self.delegate searchDLNAResult:[deviceMarr copy]];
    }
    self.dataArray = deviceMarr;
}

- (void)upnpSearchErrorWithError:(NSError *)error{
//    NSLog(@"DLNA_Error======>%@", error);
}

#pragma mark - CLUPnPResponseDelegate
- (void)upnpSetAVTransportURIResponse{
    [self.render play];
    [[NSNotificationCenter defaultCenter] postNotificationName:kMRDLNASetAVTransportURINotification object:nil];
}

- (void)upnpGetTransportInfoResponse:(CLUPnPTransportInfo *)info{
//    NSLog(@"%@ === %@", info.currentTransportState, info.currentTransportStatus);
    NSString *state = [info.currentTransportState lowercaseString];
    if (!([state isEqualToString:@"playing"] || [info.currentTransportState isEqualToString:@"transitioning"])) {
        if (![info.currentTransportState isEqualToString:@"paused_playback"]) {
            [self.render play];
        }
    }
    self.transportState = state;
    [[NSNotificationCenter defaultCenter] postNotificationName:kMRDLNAGetTransportInfoResponseNotification object:info];
}

- (void)upnpPlayResponse{
    if ([self.delegate respondsToSelector:@selector(dlnaStartPlay)]) {
        [self.delegate dlnaStartPlay];
    }
    [[NSNotificationCenter defaultCenter] postNotificationName:kMRDLNAPlayResponseNotification object:nil];
}

- (void)upnpNextResponse {
    [[NSNotificationCenter defaultCenter] postNotificationName:kMRDLNANextResponseNotification object:nil];
}

- (void)upnpSeekResponse {
    [[NSNotificationCenter defaultCenter] postNotificationName:kMRDLNASeekResponseNotification object:nil];
}

- (void)upnpStopResponse {
    [[NSNotificationCenter defaultCenter] postNotificationName:kMRDLNAStopResponseNotification object:nil];
}

- (void)upnpPauseResponse {
    [[NSNotificationCenter defaultCenter] postNotificationName:kMRDLNAPauseResponseNotification object:nil];
}

- (void)upnpPreviousResponse {
    [[NSNotificationCenter defaultCenter] postNotificationName:kMRDLNAPreviousResponseNotification object:nil];
}

- (void)upnpSetVolumeResponse {
    [[NSNotificationCenter defaultCenter] postNotificationName:kMRDLNASetVolumeResponseNotification object:nil];
}

- (void)upnpGetVolumeResponse:(NSString *)volume {
    [[NSNotificationCenter defaultCenter] postNotificationName:kMRDLNAGetVolumeResponseNotification object:volume];
}

- (void)upnpGetPositionInfoResponse:(CLUPnPAVPositionInfo *)info {
    [[NSNotificationCenter defaultCenter] postNotificationName:kMRDLNAGetPositionInfoResponseNotification object:info];
}

- (void)upnpSetNextAVTransportURIResponse {
    [[NSNotificationCenter defaultCenter] postNotificationName:kMRDLNASetNextAVTransportURIResponseNotification object:nil];
}

- (void)upnpUndefinedResponse:(NSString *)resXML postXML:(NSString *)postXML {
    NSMutableDictionary *dict = @{}.mutableCopy;
    dict[@"resXML"] = resXML;
    dict[@"postXML"] = postXML;
    [[NSNotificationCenter defaultCenter] postNotificationName:kMRDLNAUndefinedResponseNotification object:dict];
}

#pragma mark Set&Get
- (void)setSearchTime:(NSInteger)searchTime{
    _searchTime = searchTime;
    self.upd.searchTime = searchTime;
}
@end
