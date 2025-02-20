/* Copyright (c) 2008 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// These node, element, and document classes implement a subset of the methods
// provided by NSXML.  While NSXML behavior is mimicked as much as possible,
// there are important differences.
//
// The biggest difference is that, since this is based on libxml2, there
// is no retain model for the underlying node data.  Rather than copy every
// node obtained from a parse tree (which would have a substantial memory
// impact), we rely on weak references, and it is up to the code that
// created a document to retain it for as long as any
// references rely on nodes inside that document tree.


#import <Foundation/Foundation.h>

// libxml includes require that the target Header Search Paths contain
//
//   /usr/include/libxml2
//
// and Other Linker Flags contain
//
//   -lxml2

// <>引头文件其实是libxml是一个文件夹,tree.h是文件夹中的文件
// 第一步:添加系统或者第三方库文件
// 第二步:找文件夹的路径
// 第三步:添加头文件路径,在BuildSetting->header search path,添加/usr/include/libxml2 路径
// header search path 代表的是添加的库文件的.h文件所在的位置,以后的第三方中会经常使用到,比如 百度地图
#import <libxml/tree.h>
#import <libxml/parser.h>
#import <libxml/xmlstring.h>
#import <libxml/xpath.h>
#import <libxml/xpathInternals.h>


#if (MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_4) || defined(GDATA_TARGET_NAMESPACE)
  // we need NSInteger for the 10.4 SDK, or we're using target namespace macros
  #import "GDataDefines.h"
#endif

#undef _EXTERN
#undef _INITIALIZE_AS
#ifdef GDATAXMLNODE_DEFINE_GLOBALS
#define _EXTERN
#define _INITIALIZE_AS(x) =x
#else
#define _EXTERN extern
#define _INITIALIZE_AS(x)
#endif

// when no namespace dictionary is supplied for XPath, the default namespace
// for the evaluated tree is registered with the prefix _def_ns
_EXTERN const char* kXLDataXMLXPathDefaultNamespacePrefix _INITIALIZE_AS("_def_ns");

// Nomenclature for method names:
//
// Node = GData node
// XMLNode = xmlNodePtr
//
// So, for example:
//  + (id)nodeConsumingXMLNode:(xmlNodePtr)theXMLNode;

@class NSArray, NSDictionary, NSError, NSString, NSURL;
@class XLDataXMLElement, XLDataXMLDocument;

enum {
  GDataXMLInvalidKind = 0,
  GDataXMLDocumentKind,
  GDataXMLElementKind,
  GDataXMLAttributeKind,
  GDataXMLNamespaceKind,
  GDataXMLProcessingInstructionKind,
  GDataXMLCommentKind,
  GDataXMLTextKind,
  GDataXMLDTDKind,
  GDataXMLEntityDeclarationKind,
  GDataXMLAttributeDeclarationKind,
  GDataXMLElementDeclarationKind,
  GDataXMLNotationDeclarationKind
};

typedef NSUInteger GDataXMLNodeKind;

@interface XLDataXMLNode : NSObject {
@protected
  // NSXMLNodes can have a namespace URI or prefix even if not part
  // of a tree; xmlNodes cannot.  When we create nodes apart from
  // a tree, we'll store the dangling prefix or URI in the xmlNode's name,
  // like
  //   "prefix:name"
  // or
  //   "{http://uri}:name"
  //
  // We will fix up the node's namespace and name (and those of any children)
  // later when adding the node to a tree with addChild: or addAttribute:.
  // See fixUpNamespacesForNode:.

  xmlNodePtr xmlNode_; // may also be an xmlAttrPtr or xmlNsPtr
  BOOL shouldFreeXMLNode_; // if yes, xmlNode_ will be free'd in dealloc

  // cached values
  NSString *cachedName_;
  NSArray *cachedChildren_;
  NSArray *cachedAttributes_;
}

+ (XLDataXMLElement *)elementWithName:(NSString *)name;
+ (XLDataXMLElement *)elementWithName:(NSString *)name stringValue:(NSString *)value;
+ (XLDataXMLElement *)elementWithName:(NSString *)name URI:(NSString *)value;

+ (id)attributeWithName:(NSString *)name stringValue:(NSString *)value;
+ (id)attributeWithName:(NSString *)name URI:(NSString *)attributeURI stringValue:(NSString *)value;

+ (id)namespaceWithName:(NSString *)name stringValue:(NSString *)value;

+ (id)textWithStringValue:(NSString *)value;

- (NSString *)stringValue;
- (void)setStringValue:(NSString *)str;

- (NSUInteger)childCount;
- (NSArray *)children;
- (XLDataXMLNode *)childAtIndex:(unsigned)index;

- (NSString *)localName;
- (NSString *)name;
- (NSString *)prefix;
- (NSString *)URI;

- (GDataXMLNodeKind)kind;

- (NSString *)XMLString;

+ (NSString *)localNameForName:(NSString *)name;
+ (NSString *)prefixForName:(NSString *)name;

// This is the preferred entry point for nodesForXPath.  This takes an explicit
// namespace dictionary (keys are prefixes, values are URIs).
- (NSArray *)nodesForXPath:(NSString *)xpath namespaces:(NSDictionary *)namespaces error:(NSError **)error;

// This implementation of nodesForXPath registers namespaces only from the
// document's root node.  _def_ns may be used as a prefix for the default
// namespace, though there's no guarantee that the default namespace will
// be consistenly the same namespace in server responses.
- (NSArray *)nodesForXPath:(NSString *)xpath error:(NSError **)error;

// access to the underlying libxml node; be sure to release the cached values
// if you change the underlying tree at all
- (xmlNodePtr)XMLNode;
- (void)releaseCachedValues;

@end


@interface XLDataXMLElement : XLDataXMLNode

- (id)initWithXMLString:(NSString *)str error:(NSError **)error;

- (NSArray *)namespaces;
- (void)setNamespaces:(NSArray *)namespaces;
- (void)addNamespace:(XLDataXMLNode *)aNamespace;

- (void)addChild:(XLDataXMLNode *)child;
- (void)removeChild:(XLDataXMLNode *)child;

- (NSArray *)elementsForName:(NSString *)name;
- (NSArray *)elementsForLocalName:(NSString *)localName URI:(NSString *)URI;

- (NSArray *)attributes;
- (XLDataXMLNode *)attributeForName:(NSString *)name;
- (XLDataXMLNode *)attributeForLocalName:(NSString *)name URI:(NSString *)attributeURI;
- (void)addAttribute:(XLDataXMLNode *)attribute;

- (NSString *)resolvePrefixForNamespaceURI:(NSString *)namespaceURI;

@end

@interface XLDataXMLDocument : NSObject {
@protected
  xmlDoc* xmlDoc_; // strong; always free'd in dealloc
}

- (id)initWithXMLString:(NSString *)str options:(unsigned int)mask error:(NSError **)error;
- (id)initWithData:(NSData *)data options:(unsigned int)mask error:(NSError **)error;
- (id)initWithRootElement:(XLDataXMLElement *)element;

- (XLDataXMLElement *)rootElement;

- (NSData *)XMLData;

- (void)setVersion:(NSString *)version;
- (void)setCharacterEncoding:(NSString *)encoding;

// This is the preferred entry point for nodesForXPath.  This takes an explicit
// namespace dictionary (keys are prefixes, values are URIs).
- (NSArray *)nodesForXPath:(NSString *)xpath namespaces:(NSDictionary *)namespaces error:(NSError **)error;

// This implementation of nodesForXPath registers namespaces only from the
// document's root node.  _def_ns may be used as a prefix for the default
// namespace, though there's no guarantee that the default namespace will
// be consistenly the same namespace in server responses.
- (NSArray *)nodesForXPath:(NSString *)xpath error:(NSError **)error;

- (NSString *)description;
@end
