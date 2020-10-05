#import <ob/OBString.h>

@class OBURL;

@interface WkHitTest : OBObject

- (OBString *)selectedText;
- (OBString *)title;
- (OBString *)altText;
- (OBString *)textContent;

- (BOOL)isContentEditable;

- (OBURL *)linkURL;

// Image may have no URL, so the getter is here to make copy 2 clip, etc possible
- (BOOL)isImage;
- (OBURL *)imageURL;
- (OBString *)imageFileExtension;
- (OBString *)imageMimeType;
- (LONG)imageWidth;
- (LONG)imageHeight;

// Helpers
- (void)downloadLinkFile;
- (void)downloadImageFile;
- (BOOL)copyImageToClipboard;
- (BOOL)saveImageToFile:(OBString *)path;
- (void)replaceSelectedTextWidth:(OBString *)replacement;
- (void)cutSelectedText;
- (void)pasteText;
- (void)selectAll;

@end
