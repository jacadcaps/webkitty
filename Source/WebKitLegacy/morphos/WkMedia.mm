#import "WkMedia_private.h"
#import <ob/OBURL.h>
#import <ob/OBArrayMutable.h>

static inline ULONG _hash(ULONG x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

@implementation WkMediaObjectPrivate

- (id)initWithType:(WkMediaObjectType)type identifier:(WkWebViewMediaIdentifier)identifier
	audioTrack:(id<WkWebViewAudioTrack>)audioTrack videoTrack:(id<WkWebViewVideoTrack>)videoTrack downloadableURL:(OBURL *)url
{
	if ((self = [super init]))
	{
		_tracks = [OBMutableArray new];
		_audioTrack = [audioTrack retain];
		_videoTrack = [videoTrack retain];
		[_tracks addObject:audioTrack];
		[_tracks addObject:videoTrack];
		_identifier = [(id<WkMediaObjectComms>)identifier retain];
		_type = type;
		_url = [url retain];
	}
	
	return self;
}

- (void)dealloc
{
	[_audioTrack release];
	[_videoTrack release];
	[_url release];
	[_identifier release];
	[_tracks release];
	[super dealloc];
}

- (BOOL)isEqual:(id)otherObject
{
	if (otherObject == self)
		return YES;
	if ([otherObject isKindOfClass:[self class]])
	{
		WkMediaObjectPrivate *other = otherObject;
		if (other->_identifier == _identifier)
			return YES;
	}
	return NO;
}

- (ULONG)hash
{
	return _hash((ULONG)_identifier);
}

- (id<WkWebViewVideoTrack>)videoTrack
{
	return _videoTrack;
}

- (id<WkWebViewAudioTrack>)audioTrack
{
	return _audioTrack;
}

- (OBArray *)allAudioTracks
{
	return nil;
}

- (OBArray *)allVideoTracks
{
	return nil;
}

- (OBArray *)allTracks
{
	return _tracks;
}

- (WkMediaObjectType)type
{
	return _type;
}

- (WkWebViewMediaIdentifier)identifier
{
	return (WkWebViewMediaIdentifier)_identifier;
}

- (OBURL *)downloadableURL
{
	return _url;
}

- (BOOL)playing
{
	return NO;
}

- (void)play
{
	[_identifier play];
}

- (void)pause
{
	[_identifier pause];
}

- (void)setMuted:(BOOL)muted
{
	[_identifier setMuted:muted];
}

- (BOOL)muted
{
	return [_identifier muted];
}

- (void)addTrack:(id<WkWebViewMediaTrack>)track
{
	[_tracks addObject:track];
}

- (void)removeTrack:(id<WkWebViewMediaTrack>)track
{
	[_tracks removeObject:track];

	if (_audioTrack == track)
	{
		[_audioTrack release];
		_audioTrack = nil;
	}
	else if (_videoTrack == track)
	{
		[_videoTrack release];
		_videoTrack = nil;
	}
}

- (void)selectTrack:(id<WkWebViewMediaTrack>)track
{
	if ([track type] == WkWebViewMediaTrackType_Audio)
	{
		[_audioTrack autorelease];
		_audioTrack = (id)[track retain];
	}
	else
	{
		[_videoTrack autorelease];
		_videoTrack = (id)[track retain];
	}
}

@end

@implementation WkWebViewVideoTrackPrivate

- (id)initWithCodec:(OBString *)codec width:(int)width height:(int)height bitrate:(int)bitrate
{
	if ((self = [super init]))
	{
		_codec = [codec copy];
		_width = width;
		_height = height;
		_bitrate = bitrate;
	}
	
	return self;
}

- (void)dealloc
{
	[_codec release];
	[super dealloc];
}

- (OBString *)codec
{
	return _codec;
}

- (int)width
{
	return _width;
}

- (int)height
{
	return _height;
}

- (int)bitrate
{
	return _bitrate;
}

- (WkWebViewMediaTrackType)type
{
	return WkWebViewMediaTrackType_Video;
}

@end

@implementation WkWebViewAudioTrackPrivate

- (id)initWithCodec:(OBString *)codec frequency:(int)freq channels:(int)channels bits:(int)bpc
{
	if ((self = [super init]))
	{
		_codec = [codec retain];
		_frequency = freq;
		_channels = channels;
		_bits = bpc;
	}
	
	return self;
}

- (void)dealloc
{
	[_codec release];
	[super dealloc];
}

- (OBString *)codec
{
	return _codec;
}

- (int)frequency
{
	return _frequency;
}

- (int)channels
{
	return _channels;
}

- (int)bits
{
	return _bits;
}

- (WkWebViewMediaTrackType)type
{
	return WkWebViewMediaTrackType_Audio;
}

@end
