# EECS 482 Project 4: Network File System

- Due Date: April 18, 2022

## Team Members

- Binhao Qin ([bhqin](mailto:bhqin@umich.edu))
- Gil Dhason ([adhason](mailto:adhason@umich.edu))
- Weili Li ([weilirs](mailto:weilirs@umich.edu))

## Schedule

- [x] Parsing (April 1)

- [x] File System Planning (April 2)

- [x] File system implementation (April 15)
  - [x] Read
  - [x] Write
  - [x] Create
  - [x] Delete

- [x] Testing (April 18)

## Notes and Q&A

### Misc

- Take care of output, perhaps a dedicated thread or class that does that

### Data Structures to Maintain

- A list of free blocks

- Ordered operation of write requests (create, write, delete)

- An array of locks

### Development Notes

- A base class for all requests on file system
  - Need find target block
  - Need deal with socket
  - Need take care of common error checking
  - Need easy way of navigating in directory entries

- 4 functor class for each type of request

- Add path name tokenization to `Reqeust`

## Workflow

1. Add header files under `include` and add source files under `src`.

2. Run `make syntax` before adding and commiting code and fix any error.

3. Use `git add -u` and `git add <file>`, don't do `git add .`.

4. Don't commit configuration files, temp files and build files. You can use `.gitignore` to prevent this.

5. Do a `git pull` or `git pull --rebase` before pushing.
