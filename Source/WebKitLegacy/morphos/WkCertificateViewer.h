#import <mui/MUIGroup.h>
#import "WkCertificate.h"

@class MCCListtree, MUIText, MUIList;

@interface WkCertificateVerifier : MUIGroup
{
	WkCertificateChain *_certificateChain;
	MCCListtree        *_tree;
	MUIText            *_logo;
	MUIText            *_name;
	MUIText            *_issuedBy;
	MUIText            *_expires;
	MUIList            *_details;
	MUIText            *_valid;
}

+ (WkCertificateVerifier *)verifierForCertificateChain:(WkCertificateChain *)chain;

- (WkCertificateChain *)certificateChain;

// Allows overloading the event, must call super to allow for regular processing
- (void)onCertificateSelected:(WkCertificate *)certificate;

@end
