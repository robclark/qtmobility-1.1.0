/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in 
** accordance with the Qt Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/
// Copyright (c) 2008 Roberto Raggi <roberto.raggi@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef CPLUSPLUS_CPLUSPLUSFORWARDDECLARATIONS_H
#define CPLUSPLUS_CPLUSPLUSFORWARDDECLARATIONS_H

#include <cstdlib>
#include <cstddef>

#ifndef CPLUSPLUS_WITHOUT_QT
#  include <QtCore/qglobal.h>

#  if defined(CPLUSPLUS_BUILD_LIB)
#    define CPLUSPLUS_EXPORT Q_DECL_EXPORT
#  elif defined(CPLUSPLUS_BUILD_STATIC_LIB)
#    define CPLUSPLUS_EXPORT
#  else
#    ifdef ICHECK_BUILD
#      define CPLUSPLUS_EXPORT
#    else
#      define CPLUSPLUS_EXPORT Q_DECL_IMPORT
#    endif
#  endif
#else
#  define CPLUSPLUS_EXPORT
#endif

namespace CPlusPlus {

class TranslationUnit;
class Semantic;
class Control;
class MemoryPool;
class DiagnosticClient;

class Identifier;
class Literal;
class StringLiteral;
class NumericLiteral;

class Scope;
class TemplateParameters;

// names
class NameVisitor;
class Name;
class NameId;
class TemplateNameId;
class DestructorNameId;
class OperatorNameId;
class ConversionNameId;
class QualifiedNameId;
class SelectorNameId;

// types
class TypeMatcher;
class FullySpecifiedType;
class TypeVisitor;
class Type;
class UndefinedType;
class VoidType;
class IntegerType;
class FloatType;
class PointerToMemberType;
class PointerType;
class ReferenceType;
class ArrayType;
class NamedType;

// symbols
class SymbolVisitor;
class Symbol;
class ScopedSymbol;
class UsingNamespaceDirective;
class UsingDeclaration;
class Declaration;
class Argument;
class TypenameArgument;
class Function;
class Namespace;
class BaseClass;
class Block;
class Class;
class Enum;
class ForwardClassDeclaration;

class Token;

// Objective-C symbols
class ObjCBaseClass;
class ObjCBaseProtocol;
class ObjCClass;
class ObjCForwardClassDeclaration;
class ObjCProtocol;
class ObjCForwardProtocolDeclaration;
class ObjCMethod;
class ObjCPropertyDeclaration;

} // end of namespace CPlusPlus

#endif // CPLUSPLUS_CPLUSPLUSFORWARDDECLARATIONS_H
