# Project Explanation

## Set up the Base (N) and Letter Limit (M)
You first assume your Howard ID (a decimal number, for example, `12345678`). You compute:

1. **Remainder (`X % 11`)**: Divide your Howard ID by `11` and take the remainder.
2. **N**: Add `26` to that remainder (i.e., `N = 26 + (X % 11)`). This value (which will be between `26` and `36`) is the base you will use.
3. **M**: Subtract `10` from `N` (i.e., `M = N – 10`). This value tells you how many letters (starting from `'a'` or `'A'`) are valid.  
   - In other words, the valid lowercase letters run from `'a'` to the `M`-th letter (β), and the valid uppercase letters run from `'A'` to the `M`-th uppercase letter (Δ).

For instance, if your Howard ID is `12345678`, then:

- `12345678 % 11 = 4`
- `N = 30`
- `M = 20`

This means valid letters are `'a'` to `'t'` (or `'A'` to `'T'`), and they represent the values `10` through `29`.

---

## Input Processing
Your program will read a string that is exactly **10 characters long** (spaces count as characters).  
(Any newline or null terminator that might come with the input is to be ignored.)

---

## Valid Digit Extraction and Conversion
For each character in the string:

- **If it is a digit (`'0'-'9'`)**: Its value is the usual numeric value (`0–9`).
- **If it is a letter**:
  - Check if it lies within the valid range.  
    - For lowercase, that means from `'a'` up to the `M`-th letter (e.g., if `M = 20`, then from `'a'` to `'t'`).
    - Similarly, for uppercase letters, valid ones are from `'A'` to the `M`-th uppercase letter.
  - For either case, the letter (whether lowercase or uppercase) represents a digit with the value:
    - `10` for `'a'`/`'A'`
    - `11` for `'b'`/`'B'`
    - …
    - `(N – 1)` for the last valid letter.
- **If a character does not meet any of these criteria**, it is simply ignored.

---

## Summing and Output
1. Split the **10 characters** into two halves:
   - The **first 5** (`G`)
   - The **last 5** (`H`)
2. Sum up the numeric values of valid digits found in the first half (call this sum **G**) and sum those found in the second half (call this sum **H**).
3. Compute the difference:  
   **`G – H`** and print this result as a decimal integer.
4. If **no valid digit** is found anywhere in the input, print exactly:  
   **`N/A`**.

---

## Program Termination
After processing one input (one string), the program exi.
