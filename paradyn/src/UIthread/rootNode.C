// rootNode.C
// Ariel Tamches

/* $Log: rootNode.C,v $
/* Revision 1.2  1995/07/18 03:41:19  tamches
/* Added ctrl-double-click feature for selecting/unselecting an entire
/* subtree (nonrecursive).  Added a "clear all selections" option.
/* Selecting the root node now selects the entire program.
/*
 * Revision 1.1  1995/07/17  04:58:57  tamches
 * First version of the new where axis
 *
 */

#include <assert.h>

#include "rootNode.h"

int rootNode::borderPix = 3;
int rootNode::horizPad = 3;
int rootNode::vertPad = 2;

void rootNode::manual_construct(const char *init_str, const where4TreeConstants &tc,
				const bool init_highlighted) {
   name = init_str; // makes a copy
   highlighted = init_highlighted;
   pixWidth = wouldbe_width(tc);
   pixHeight = wouldbe_height(tc);
}

rootNode::rootNode(const string &init_str, const where4TreeConstants &tc, const bool hilited) {
   manual_construct(init_str.string_of(), tc, hilited);
}

rootNode::rootNode(const char *init_str, const where4TreeConstants &tc, const bool hilited) {
      manual_construct(init_str, tc, hilited);
}

rootNode::~rootNode() {
}

const string &rootNode::getName() const { return name; }

int rootNode::getWidth() const { return pixWidth; }
int rootNode::getHeight() const { return pixHeight; }
bool rootNode::getHighlighted() const { return highlighted; }

int rootNode::wouldbe_width(const where4TreeConstants &tc) const {
   // A relatively expensive routine; don't call too often.
   const int textWidth = XTextWidth(tc.rootTextFontStruct,
				    name.string_of(),
				    name.length());
   return borderPix + horizPad + textWidth + horizPad + borderPix;
}

int rootNode::wouldbe_height(const where4TreeConstants &tc) const {
   return borderPix + vertPad +
          tc.rootTextFontStruct->ascent + tc.rootTextFontStruct->descent +
          vertPad + borderPix;
}

void rootNode::draw(const where4TreeConstants &tc,
		    int theDrawable,
	            const int root_middlex, const int root_topy) const {
   const int boxLeft = root_middlex - (pixWidth / 2);

   // First, some quick & dirty clipping:
   const int minWindowX = 0;
   const int maxWindowX = Tk_Width(tc.theTkWindow) - 1;
   const int minWindowY = 0;
   const int maxWindowY = Tk_Height(tc.theTkWindow) - 1;

   if (root_topy > maxWindowY)
      return;
   if (root_topy + pixHeight - 1 < minWindowY)
      return;
   if (boxLeft > maxWindowX)
      return;
   if (boxLeft + pixWidth - 1 < minWindowX)
      return;

   // Let's try the tk 3d thing:
//   const int normalRelief = TK_RELIEF_RAISED;
//   const int highlightedRelief = TK_RELIEF_RIDGE;

//   const int normalRelief = TK_RELIEF_RAISED;
//   const int highlightedRelief = TK_RELIEF_SUNKEN;

   const int normalRelief = TK_RELIEF_GROOVE;
   const int highlightedRelief = TK_RELIEF_SUNKEN;

   Tk_Fill3DRectangle(tc.theTkWindow,
		      theDrawable,
		      tc.rootNodeBorder,
		      boxLeft, root_topy,
		      pixWidth, pixHeight,
		      borderPix,
		      highlighted ? highlightedRelief : normalRelief
		      );

   // Third, draw the text
   int textAscentHeight = tc.rootTextFontStruct->ascent;
   const int textLeft = boxLeft + borderPix + horizPad;
   const int textBaseLine = root_topy + borderPix + vertPad + textAscentHeight - 1;

   XDrawString(tc.display, theDrawable,
	       tc.rootItemTextGC,
	       textLeft, textBaseLine,
	       name.string_of(), name.length());
}

int rootNode::pointWithin(const int xpix, const int ypix,
			  const int root_centerx, const int root_topy) const {
   // return values:
   // 1 -- yes
   // 2 -- no, point is above the root
   // 3 -- no, point is below root
   // 4 -- no, point is to the left of root
   // 5 -- no, point is to the right of root
   
   assert(xpix >= 0 && ypix >= 0);
   
   if (ypix < root_topy) return 2;

   const int root_bottomy = root_topy + pixHeight - 1;
   if (ypix > root_bottomy) return 3;

   const int root_leftx = root_centerx - pixWidth / 2;
   if (xpix < root_leftx) return 4;

   const int root_rightx = root_leftx + pixWidth - 1;
   if (xpix > root_rightx) return 5;

   assert(xpix >= root_leftx && xpix <= root_rightx);
   assert(ypix >= root_topy && ypix <= root_bottomy);
   return 1; // bingo
}

void rootNode::toggle_highlight(const bool redrawNow,
				const where4TreeConstants &tc,
				const int middlex, const int topy) {
   highlighted = !highlighted;
   if (redrawNow)
      draw(tc, tc.masterWindow, middlex, topy);
}

void rootNode::toggle_highlight() {
   highlighted = !highlighted;
}

void rootNode::unhighlight(const bool redrawNow,
			   const where4TreeConstants &tc,
			   const int middlex, const int topy) {
   highlighted = false;

   if (redrawNow)
      draw(tc, tc.masterWindow, middlex, topy);
}

void rootNode::highlight() {
   highlighted = true;
}

void rootNode::unhighlight() {
   highlighted = false;
}

