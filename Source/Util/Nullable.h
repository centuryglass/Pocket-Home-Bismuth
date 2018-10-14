#pragma once

/**
 * @file Nullable.h
 *
 * @brief  A template for classes that may or may not hold data.
 *
 * Nullable object containers are intended to package abstract pointer types.
 *
 * Each Nullable class has a specific data pointer type.  Nullable classes
 * copy the interface of their data object, forwarding all function calls to
 * their object data pointer.  For each copied interface method, the nullable
 * class must check if its data pointer is null, and handle the problem 
 * appropriately if it is.
 *
 * This pattern provides all of the convenience of polymorphic data pointers, 
 * without the hassle of debugging segmentation faults or manually managing
 * memory.  In circumstances where a function returning a default value or 
 * occasionally being no-op would be inappropriate, Nullable objects should use
 * exceptions to ensure that unchecked function calls can be easily found and
 * fixed.
 *
 * @tparam PtrType  The type of data pointer held by the Nullable object.
 *                  This may be a normal pointer, an std::unique_ptr,
 *                  a juce::ReferenceCountedObjectPtr, or any sort of 
 *                  similar value that might equal nullptr.
 */

template<class PtrType>
class Nullable
{
protected:
    /**
     * @brief  Creates a Nullable containing a data pointer.
     *
     * @param dataPtr  The object's data pointer.
     */
    Nullable(PtrType dataPtr) : dataPtr(dataPtr) { }

    /**
     * @brief  Creates a null object containing no data.
     */
    Nullable() : dataPtr(nullptr) { }

public:
    virtual ~Nullable() { }

    /**
     * @brief  Checks if this object's data pointer is null.
     *
     * @return  True if the data pointer equals nullptr, false if it points to
     *          a data object.
     */
    bool isNull() const
    {
        return dataPtr == nullptr;
    }

protected:
    /**
     * @brief  Gets this object's data pointer.
     *
     * @return  The data pointer's value. 
     */
    PtrType getData() const
    {
        return dataPtr;
    }

    /**
     * @brief  Accesses the object's data pointer.
     *
     * @return  A reference to the internal data pointer.
     */
    PtrType& getDataReference()
    {
        return dataPtr;
    }

private:
    /* Holds a pointer to object data that might be null. */
    PtrType dataPtr;
};
