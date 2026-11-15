from random import shuffle


def basic_bubble_sort(nums):
    for i in range(len(nums)):
        for j in range(len(nums) - i - 1):
            if nums[j] > nums[j + 1]:
                nums[j], nums[j + 1] = nums[j + 1], nums[j]
        print(f"Sort {i}:", nums)

def bubble_sort_check_ending_continuity(nums):
    k = len(nums) - 1
    swapped = False
    for j in range(k):
        if nums[j] > nums[j + 1]:
            nums[j], nums[j + 1] = nums[j + 1], nums[j]
            swapped = True
    if not swapped:
        print(f"Not swapped! Contiunue = {k + 1}")
        return
    last = nums[k]
    print("First sort:", nums, f"last = {last}")
    for i in range(1, k):
        last -= 1
        if last == nums[k - i]:
            print("Skip!")
            continue
        for j in range(k - i):
            if nums[j] > nums[j + 1]:
                nums[j], nums[j + 1] = nums[j + 1], nums[j]
        print(f"Sort {i}:", nums)
        print(f"Last one is {nums[k - i]}, last = {last}")
        if last != nums[k - i]:
            print(f"Discontinuous! Continue: {i}")
            return

nums = list(range(1, 11))
shuffle(nums)
print("Initial:", nums)
basic_bubble_sort(nums)
print("\n\n" + "#" * 50)

nums = list(range(1, 21))
nums.remove(10)
shuffle(nums)
print("Initial:", nums)
bubble_sort_check_ending_continuity(nums)
print("\n\n" + "#" * 50)

