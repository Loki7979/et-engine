/*
 * This file is part of `et engine`
 * Copyright 2009-2012 by Sergey Reznik
 * Please, do not modify contents without approval.
 *
 */

#include <UIKit/UIPrintInteractionController.h>
#include <et/platform-ios/printer.h>

using namespace et;

Printer::Printer() : _private(nullptr)
{
}

Printer::~Printer()
{
}

void Printer::printImageFromFile(const std::string& s)
{
	UIImage* image = [UIImage imageWithContentsOfFile:[NSString stringWithUTF8String:s.c_str()]];
	[[UIPrintInteractionController sharedPrintController] setPrintingItem:image];
	[[UIPrintInteractionController sharedPrintController] presentAnimated:YES completionHandler:nil];
}