from random import shuffle
from copy   import deepcopy


def bubble_sort_check_ending_continuity_quick(nums):
    end = len(nums) - 1
    while end > 0:
        last_swap = 0
        for j in range(end):
            if nums[j] > nums[j + 1]:
                nums[j], nums[j + 1] = nums[j + 1], nums[j]
                last_swap = j
        end = last_swap
        print(nums)


def insertion_sort(nums):
    for i in range(1, len(nums)):
        k = nums[i]
        j = i
        while j > 0 and nums[j - 1] > k:
            nums[j] = nums[j - 1]
            j -= 1
        nums[j] = k
        print(nums)
    
nums = list(range(1, 21))
nums.remove(10)
shuffle(nums)

nums1 = deepcopy(nums)
print("Initial:", nums1)
bubble_sort_check_ending_continuity_quick(nums1)
print("\n\n" + "#" * 50)

nums2 = deepcopy(nums)
print("Initial:", nums2)
insertion_sort(nums2)
