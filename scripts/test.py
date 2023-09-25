def bs(array, target):
    n = len(array)
    l=0
    r=n
    while l<r:
        k = l + (r-l) // 2
        if array[k] == target:
            return True
        elif array[k] > target:
            r = k
        else:
            l=k+1


    return False


arr = [1,2,3,4,5,6,6,7,8,9,12]


assert(bs(arr, -1) == False)
assert(bs(arr, 0) == False)
assert(bs(arr, 1) == True)
assert(bs(arr, 6) == True)
assert(bs(arr, 10) == False)
assert(bs(arr, 12) == True)
assert(bs(arr, 13) == False)