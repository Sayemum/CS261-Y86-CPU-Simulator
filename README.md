**Y86 CPU Simulator**

**CONTEXT**

The x86 architecture is a complex instruction set architecture. Intel initially developed it based on the Intel 8086 microprocessor and its 8088 variant.
The x86 architecture is relevant to computers because it is the foundation of most personal computers and servers in use today. It is a versatile and powerful architecture that has been refined over the years to provide better performance and more features.
The x86 architecture is also compatible with a wide range of software, making it a popular choice for developers and users alike.

The x86 architecture is vastly complex with many intricate components embedded within the architecture. Therefore I decided to replicate this architecture using a miniature-sized version called Y86 with less and simpler instruction sets.

Let's look at an example:

![image](https://github.com/Sayemum/CS261-Y86-CPU-Simulator/assets/84354582/7075c405-3468-46e5-99ef-7c644600b572)

The above image shows a set of instructions written in assembly language. The program will read through each instruction and interpret them based on the von Neumann architecture and make changes to our virtual CPU. With each instruction, it has the possibility of changing register values, setting flags,
changing CPU states, etc. But let's take a look at how these are being done on the console.

![image](https://github.com/Sayemum/CS261-Y86-CPU-Simulator/assets/84354582/868f3748-ec83-41d6-8473-74fe465540ca)

The above image shows the changes being made using trace mode to see the changes to the CPU as we move along with each instruction.

![image](https://github.com/Sayemum/CS261-Y86-CPU-Simulator/assets/84354582/23e987f4-0470-4fa2-b349-49b7d9ebd51e)

Finally, we get to see the contents of virtual memory. It's very massive due to the size of virtual memory we allocated for the CPU, which is why the image shows an abridged version of the actual results.

**CONCLUSION**

This Y86 Miniature CPU took a whole semester to create and it demonstrates the complexities of instruction set architecture.
