//
//  MRDLNA.h
//  MRDLNA
//
//  Created by MccRee on 2018/5/4.
//

#import <Foundation/Foundation.h>
#import "CLUPnP.h"
#import "CLUPnPDevice.h"

//- (void)upnpSetAVTransportURIResponse;  // 设置url响应
//- (void)upnpUndefinedResponse:(NSString *)resXML postXML:(NSString *)postXML;
//
//- (void)upnpPlayResponse;                   // 播放响应
//- (void)upnpPauseResponse;                  // 暂停响应
//- (void)upnpStopResponse;                   // 停止投屏
//- (void)upnpSeekResponse;                   // 跳转响应
//- (void)upnpPreviousResponse;               // 以前的响应
//- (void)upnpNextResponse;                   // 下一个响应
//- (void)upnpSetVolumeResponse;              // 设置音量响应
//- (void)upnpSetNextAVTransportURIResponse;  // 设置下一个url响应
//- (void)upnpGetVolumeResponse:(NSString *)volume;                   // 获取音频信息
//- (void)upnpGetPositionInfoResponse:(CLUPnPAVPositionInfo *)info;   // 获取播放进度

extern NSString *const kMRDLNAGetTransportInfoResponseNotification;
extern NSString *const kMRDLNASetAVTransportURINotification;
extern NSString *const kMRDLNAPlayResponseNotification;
extern NSString *const kMRDLNAPauseResponseNotification;
extern NSString *const kMRDLNAStopResponseNotification;
extern NSString *const kMRDLNASeekResponseNotification;
extern NSString *const kMRDLNAPreviousResponseNotification;
extern NSString *const kMRDLNANextResponseNotification;
extern NSString *const kMRDLNASetVolumeResponseNotification;
extern NSString *const kMRDLNAGetVolumeResponseNotification;
extern NSString *const kMRDLNASetNextAVTransportURIResponseNotification;
extern NSString *const kMRDLNAGetPositionInfoResponseNotification;
extern NSString *const kMRDLNAUndefinedResponseNotification;


@protocol DLNADelegate <NSObject>

@optional
/**
 DLNA局域网搜索设备结果
 @param devicesArray <CLUPnPDevice *> 搜索到的设备
 */
- (void)searchDLNAResult:(NSArray *)devicesArray;


/**
 投屏成功开始播放
 */
- (void)dlnaStartPlay;



@end

@interface MRDLNA : NSObject

@property(nonatomic,weak)id<DLNADelegate> delegate;

@property(nonatomic, strong) CLUPnPDevice *device;

@property(nonatomic,copy) NSString *playUrl;

@property(nonatomic,assign) NSInteger searchTime;

@property(nonatomic,strong) CLUPnPRenderer *render;         //MDR渲染器

/**
 单例
 */
+(instancetype)sharedMRDLNAManager;

/**
 搜设备
 */
- (void)startSearch;

/**
 DLNA投屏
 */
- (void)startDLNA;
/**
 DLNA投屏(首先停止)---投屏不了可以使用这个方法
 ** 【流程: 停止 ->设置代理 ->设置Url -> 播放】
 */
- (void)startDLNAAfterStop;

/**
 退出DLNA
 */
- (void)endDLNA;

/**
 播放
 */
- (void)dlnaPlay;

/**
 暂停
 */
- (void)dlnaPause;

/**
 设置音量 volume建议传0-100之间字符串
 */
- (void)volumeChanged:(NSString *)volume;

/**
 设置播放进度 seek单位是秒
 */
- (void)seekChanged:(NSInteger)seek;

/**
 播放切集
 */
- (void)playTheURL:(NSString *)url;
@end
