package io.joynr.generator.templates.util
/*
 * !!!
 *
 * Copyright (C) 2011 - 2015 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import com.google.common.collect.Iterables
import com.google.inject.Inject
import java.util.ArrayList
import java.util.Set
import org.eclipse.emf.common.util.BasicEList
import org.eclipse.emf.common.util.EList
import org.eclipse.emf.ecore.util.EcoreUtil
import org.franca.core.franca.FArrayType
import org.franca.core.franca.FBasicTypeId
import org.franca.core.franca.FCompoundType
import org.franca.core.franca.FConstantDef
import org.franca.core.franca.FEnumerationType
import org.franca.core.franca.FEnumerator
import org.franca.core.franca.FExpression
import org.franca.core.franca.FField
import org.franca.core.franca.FInitializerExpression
import org.franca.core.franca.FIntegerConstant
import org.franca.core.franca.FInterface
import org.franca.core.franca.FMapType
import org.franca.core.franca.FModelElement
import org.franca.core.franca.FQualifiedElementRef
import org.franca.core.franca.FStringConstant
import org.franca.core.franca.FStructType
import org.franca.core.franca.FType
import org.franca.core.franca.FTypeCollection
import org.franca.core.franca.FTypeDef
import org.franca.core.franca.FTypeRef
import org.franca.core.franca.FTypedElement
import org.franca.core.franca.FUnionType

class TypeUtil {

	@Inject
	protected extension NamingUtil;

	def <T> Set<T> addElements (Set<T> originalSet, T... elements){
		for (element : elements) {
			originalSet.add(element);
		}
		return originalSet;
	}

	def String getEnumeratorValue(FExpression expression)
	{
		return switch (expression)
		{
			FIntegerConstant: expression.^val.toString
			FStringConstant: expression.^val
			FQualifiedElementRef: expression.element.constantValue
			default: null
		}
	}

	def String getConstantValue(FModelElement expression)
	{
		return switch (expression)
		{
			FConstantDef: expression.rhs.constantType
			default: null
		}
	}

	def String getConstantType(FInitializerExpression  expression)
	{
		return switch (expression)
		{
			FIntegerConstant: expression.^val.toString
			default: null
		}
	}

	def getDatatype(FType type){
		if (type instanceof FArrayType){
			return getDatatype(type.elementType)
		}
		if (type instanceof FCompoundType){
			return type
		}
		if (type instanceof FTypeDef){
			return getDatatype(type.actualType)
		}
		if (type instanceof FEnumerationType){
			return type
		}
		if (type instanceof FMapType){
			throw new IllegalStateException("JoynGeneratorExtensions.xtend: isComplex for map types is not implemented!")
//			val mapType = type as FMapType
//			return (isComplex(mapType.keyType) || isComplex(mapType.valueType));
		}

	}

	def Object getDatatype(FTypeRef typeRef) {
		if (typeRef == null){
			return null
		}

		if (typeRef.derived!=null){
			return getDatatype(typeRef.derived)
		}
		return typeRef.predefined
	}

	def boolean isLong(FBasicTypeId type) {
		return type == FBasicTypeId::INT64 || type == FBasicTypeId::UINT64;
	}

	def boolean isDouble(FBasicTypeId type) {
		return type == FBasicTypeId::DOUBLE;
	}

	def boolean isFloat(FBasicTypeId type) {
		return type == FBasicTypeId::FLOAT;
	}

	def boolean isBool(FBasicTypeId type) {
		return type == FBasicTypeId::BOOLEAN;
	}

	def boolean isShort(FBasicTypeId type) {
		return type == FBasicTypeId::INT16 || type == FBasicTypeId::UINT16;
	}

	def boolean isInteger(FBasicTypeId type) {
		return type == FBasicTypeId::INT32 || type == FBasicTypeId::UINT32;
	}

	def boolean isString(FBasicTypeId type) {
		return type == FBasicTypeId::STRING;
	}

	def boolean isByte(FBasicTypeId type) {
		return type == FBasicTypeId::UINT8 || type == FBasicTypeId::INT8;
	}

	def boolean isByteBuffer(FBasicTypeId type) {
		return type == FBasicTypeId::BYTE_BUFFER;
	}

	def boolean isByteBuffer(FTypeRef typeRef) {
		if (typeRef == null){
			return false;
		}
		return isByteBuffer(getPrimitive(typeRef))
	}

	def boolean isPrimitive(FType type){
		if (type instanceof FMapType){
			throw new IllegalStateException("JoynGeneratorExtensions.xtend: isPrimitive for map types is not implemented!")
		}
		else {
			if (type instanceof FArrayType){
				return isPrimitive(type.elementType)
			}
			if (type instanceof FTypeDef){
				return isPrimitive(type.actualType)
			}
		}
		return false;
	}

	def FBasicTypeId getPrimitive(FType type){
		if (type == null){
			return null;
		}
		if (type instanceof FArrayType){
			return getPrimitive(type.elementType)
		}
		if (type instanceof FTypeDef){
			return getPrimitive(type.actualType)
		}
	}

	def FBasicTypeId getPrimitive(FTypeRef type){
		if (type.predefined != null && type.predefined!=FBasicTypeId::UNDEFINED){
			return type.predefined
		}
		else{
			return getPrimitive(type.derived)
		}
	}

	def boolean isEnum(FTypeRef typeRef) {
		if (typeRef == null){
			return false;
		}
		if (typeRef.derived!=null){
			return isEnum(typeRef.derived)
		}
		return false
	}

	def boolean isMap(FTypeRef typeRef) {
		if (typeRef == null){
			return false;
		}
		if (typeRef.derived!=null){
			return isMap(typeRef.derived)
		}
		return false
	}

	def boolean isPrimitive(FTypeRef typeRef){
		if (typeRef== null){
			return false;
		} 

		if (typeRef.predefined!=null && typeRef.predefined!=FBasicTypeId::UNDEFINED){
			return true;
		}
		else {
			return isPrimitive(typeRef.derived)
		}
	}

	def FCompoundType getComplexType(FType type){
		if (type == null){
			return null;
		}
		if (type instanceof FArrayType){
			return getComplexType(type.elementType)
		}
		else if (type instanceof FCompoundType){
			return type;
		}
		else if (type instanceof FTypeDef){
			return getComplexType(type.actualType)
		}
	}

	def FCompoundType getComplexType(FTypeRef type){
		if (type==null){
			return null;
		}
		else{
			return getComplexType(type.derived)
		}
	}

	def FEnumerationType getEnumType(FTypeRef type){
		if (type==null){
			return null;
		}
		else{
			return getEnumType(type.derived)
		}
	}

	def FMapType getMapType(FType type){
		if (type == null){
			return null;
		}
		if (type instanceof FArrayType){
			return getMapType(type.elementType)
		}
		else if (type instanceof FMapType){
			return type;
		}
		else if (type instanceof FTypeDef){
			return getMapType(type.actualType)
		}
	}

	def FMapType getMapType(FTypeRef type){
		if (type==null){
			return null;
		}
		else{
			return getMapType(type.derived)
		}
	}

	def FEnumerationType getEnumType(FType type){
		if (type == null){
			return null;
		}
		if (type instanceof FArrayType){
			return getEnumType(type.elementType)
		}
		else if (type instanceof FEnumerationType){
			return type;
		}
		else if (type instanceof FTypeDef){
			return getEnumType(type.actualType)
		}
	}

	def boolean isComplex(FType type) {
		if (type==null){
			return false
		}
		if (type instanceof FArrayType){
			return isComplex(type.elementType)
		}
		if (type instanceof FCompoundType){
			return true
		}
		if (type instanceof FTypeDef){
			return isComplex(type.actualType)
		}
		if (type instanceof FEnumerationType){
			return false
		}
		if (type instanceof FMapType){
			throw new IllegalStateException("JoynGeneratorExtensions.xtend: isComplex for map types is not implemented!")
//			val mapType = type as FMapType
//			return (isComplex(mapType.keyType) || isComplex(mapType.valueType));
		}
		return false
	}

	def boolean isComplex(FTypeRef typeRef) {
		if (typeRef == null){
			return false;
		}
		if (typeRef.derived!=null){
			return isComplex(typeRef.derived)
		}
		return false
	}

	def boolean isMap(FType type) {
		if (type == null){
			return false;
		}
		if (type instanceof FArrayType){
			isMap(type.elementType)
		}
		if (type instanceof FTypeDef){
			isMap(type.actualType)
		}
		if (type instanceof FMapType){
			return true;
		}
		return false
	}

	def boolean isEnum(FType type) {
		if (type==null){
			return false
		}
		if (type instanceof FArrayType){
			isEnum(type.elementType)
		}
		if (type instanceof FStructType || type instanceof FUnionType){
			return false
		}
		if (type instanceof FTypeDef){
			isEnum(type.actualType)
		}
		if (type instanceof FEnumerationType){
			return true
		}
		if (type instanceof FMapType){
			throw new IllegalStateException("JoynGeneratorExtensions.xtend: isEnum for map types is not implemented!")
//			val mapType = type as FMapType
//			return (isComplex(mapType.keyType) || isComplex(mapType.valueType));
		}
		return false
	}

	def getEnumElements(FEnumerationType enumType) {
		enumType.enumerators
	}

	def EList<FEnumerator> getEnumElementsAndBaseEnumElements(FEnumerationType enumType) {
		if (hasExtendsDeclaration(enumType)) {
			val baseEnumType = getExtendedType(enumType)
			var enumAndBaseEnumElements = new BasicEList<FEnumerator>()
			val baseEnumElements = getEnumElementsAndBaseEnumElements(baseEnumType)
			enumAndBaseEnumElements.addAll(EcoreUtil.copyAll(baseEnumElements))
			val enumElements = getEnumElements(enumType)
			enumAndBaseEnumElements.addAll(EcoreUtil.copyAll(enumElements))
			return enumAndBaseEnumElements
		} else {
			return getEnumElements(enumType)
		}
	}

	def Iterable<FField> getMembersRecursive(FType datatype) {
		val datatypeInternal = getDatatype(datatype)
		val result = new ArrayList<FField>
		if (datatypeInternal instanceof FCompoundType) {
			val compoundType = datatype as FCompoundType
			result.addAll(compoundType.elements)
		}

		if (datatypeInternal instanceof FStructType && (datatypeInternal as FStructType).base!=null) {
			return Iterables::concat(getMembersRecursive((datatypeInternal as FStructType).base), result);
		} else if (datatypeInternal instanceof FUnionType && (datatypeInternal as FUnionType).base!=null) {
			return Iterables::concat(getMembersRecursive((datatypeInternal as FUnionType).base), result);
		}
		else{
			return result;
		}
	}

	def getMembers(FCompoundType compoundType) {
		compoundType.elements
	}

	def getEnumMembers(FCompoundType datatype) {
		datatype.elements.filter(element | isEnum(element.type));
	}

	def getComplexMembers(FCompoundType datatype) {
		datatype.elements.filter(element | isComplex(element.type));
	}

	def getComplexAndEnumMembers(FCompoundType datatype) {
		datatype.elements.filter(element | isComplex(element.type) || isEnum(element.type) || isArray(element));
	}

	def filterComplexAndEnum(Iterable<Object> iterable) {
		iterable.filter[type | type instanceof FType && ((type as FType).complex || (type as FType).enum) ]
	}

	def boolean isPartOfTypeCollection(FType datatype) {
		return datatype.eContainer instanceof FTypeCollection &&
			!(datatype.eContainer instanceof FInterface) &&
			(datatype.eContainer as FTypeCollection).name != "" &&
			(datatype.eContainer as FTypeCollection).name != null;
	}

	def String getTypeCollectionName(FType datatype) {
		if(!datatype.isPartOfTypeCollection) {
			throw new IllegalStateException(
					"Datatype " + datatype.joynrName + " is not part of a type collection."
					+ " Please call isPartOfTypeCollection before calling this method."
			);
		}
		return (datatype.eContainer as FTypeCollection).joynrName;
	}


	/*
	 * Returns true if the member is a 1 Dimensional list, returns false if not. (2D Lists are not supported)
	 */
	def boolean isArray(FTypedElement typedElement) {
		return typedElement.array
	}

	def boolean hasExtendsDeclaration(FCompoundType datatype) {
		if (datatype instanceof FStructType && (datatype as FStructType).base!=null) {
			return true
		} else if (datatype instanceof FUnionType && (datatype as FUnionType).base!=null) {
			return true
		}
		return false
	}

	def boolean hasExtendsDeclaration(FEnumerationType datatype) {
		if (datatype.base!=null) {
			return true
		}
		return false
	}

	def FEnumerationType getExtendedType(FEnumerationType datatype) {
		if (datatype.base!=null) {
			return datatype.base
		}
		return null
	}

	def FCompoundType getExtendedType(FCompoundType datatype) {
		if (datatype instanceof FStructType && (datatype as FStructType).base!=null) {
			return (datatype as FStructType).base
		} else if (datatype instanceof FUnionType && (datatype as FUnionType).base!=null) {
			return (datatype as FUnionType).base
		}
		return null
	}

	def String getObjectDataTypeForPlainType(String plainType) {
		var type = plainType.toLowerCase
		switch (plainType) {
			case FBasicTypeId::BOOLEAN.getName: type = "Boolean"
			case FBasicTypeId::INT8.getName: type = "Byte"
			case FBasicTypeId::UINT8.getName: type = "Byte"
			case FBasicTypeId::INT16.getName: type = "Short"
			case FBasicTypeId::UINT16.getName: type = "Short"
			case FBasicTypeId::INT32.getName: type = "Integer"
			case FBasicTypeId::UINT32.getName: type = "Integer"
			case FBasicTypeId::INT64.getName: type = "Long"
			case FBasicTypeId::UINT64.getName: type = "Long"
			case FBasicTypeId::FLOAT.getName: type = "Float"
			case FBasicTypeId::DOUBLE.getName: type = "Double"
			case FBasicTypeId::STRING.getName: type = "String"
			case FBasicTypeId::BYTE_BUFFER.getName: type = "Byte[]"
			case "void": type = "Void"
			default :  type = plainType
		}

		return type
	}
}