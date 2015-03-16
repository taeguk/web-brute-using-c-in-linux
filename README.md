Description : 

              Example codes about web brute force using c in linux.
              
              The codes are written in C/C++.
              
              Use Linux System Programming Skills to improve performance.

Files :

        1. TargetWeb.php : Simple login form written in php. Our objective is to get admin password @@@ 
        
        2. normal.c : Most Simple Web brute in C. But It is so slow. @@@ 
        
        3. thread.c :  Use thread to improve performance. Result is very good! @@@ 
        
        4. epoll.c : Use epoll to improve performance. It is a best choice. The epoll.c is better than thread.c in respect of context switching. @@@ 
        
        5. epoll_using_stl.cpp : Almost equals to epoll.c , but use C++ STL map for remembering try-password. @@@ 
        
        6. epoll_thread.c : Result of combining a idea to epoll.c , but performance is worse than epoll.c @@@ 

Result : 

          epoll.c or epoll_using_stl.cpp is almost best choice. (thread.c have almost same performance)
          
