# MultiProcess
Multiprocessos Programming Course

### OpenCL Notes

* Platform = Vendor specific OpenCL implementation
* Context = Device selected to work together
* Device = Physical device supporting OpenCL, i.e. CPU or GPU or Accelerator
* Host = Client side calling code, i.e. normal application code
* Kernel = Blueprint function which does the work
* Work Item = Unit of work executing a kernel
* Work Groups = Collection of work items
* Command Queue = Commands called on the host sent to the device for execution
* Memory = local, global, private, constant
	* Host can only access global memory!
* Buffer = Are of memory on the GPU
* Compute Unit = Work Group + Its local (private) memory


### Setting Up Project Environment

1. Solution > Properties > C/C++ > General
2. Set platform to x64
3. Additional Include Directories > Add C:\Program Files (x86)\IntelSWTools\system_studio_2020\OpenCL\sdk\include
4. Linker > Input > Additional Dependencies > Add OpenCL.lib
5. Linker > General > Additional Library Directories > Add C:\Program Files (x86)\IntelSWTools\system_studio_2020\OpenCL\sdk\lib\x64

**JUST KIDDING**
1. Create empty OpenCL Project
2. ???
3. Profit


### Example .h comment

```c
/**
    \brief Move assignment operator
    \param m ElementraySquareMatrix of which values will be assign a new one
    \tparam T IntElement or VariableElement
    \return ElementarySquareMatrix result of assignment
*/
```