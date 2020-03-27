using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using WAS.Core;

namespace WAS.DrillView.Modules
{

    public class Box
    {
        public Box()
        {

        }
        public Box(Point3d max, Point3d min)
        {
            Max = max;
            Min = min;
        }

        public bool Inside(Box _Other)
        {
            if(Min.X < _Other.Min.X &&
                Min.Y < _Other.Min.Y &&
                Min.Z < _Other.Min.Z &&
                Max.X > _Other.Max.X &&
                Max.Y > _Other.Max.Y &&
                Max.Z > _Other.Max.Z
                )
            {
                return (true);
            }

            return (false);
        }
        public Point3d Max,Min;
    }

    public class XNode
    {
        public const float ENPAND_K = 2.0f;

        public const int MAX_DEPTH = 3;
        public const int MAX_CHILD_COUNT = 10;
        public XNode()
        {
            aabb = new Box();
            children = new XNode[8];
            objects = new List<Box>();
        }

        public XNode(Box aabb,int depth)
        {
            this.aabb = aabb;
            this.depth = depth;
            children = new XNode[8];
            objects = new List<Box>();
        }

        public Box aabb;
        public XNode[] children;
        public List<Box> objects;
        public int depth;

        public bool Inside(Box box)
        {
            return aabb.Inside(box);
        }

        public bool IsSplitted()
        {
            return (children[0] != null);
        }

        public int ObjectCount()
        {
            return (objects.Count());
        }

        public int AllObjectCount()
        {
            int count = ObjectCount();
            if (IsSplitted())
            {
                for (int i = 0; i < 8; ++i)
                {
                    count += children[i].AllObjectCount();
                }
            }
            return count;
        }

        public bool add(Box box)
        {
            if(!Inside(box))
            {
                return false;
            }

            if(IsSplitted())
            {
                // 已经分裂的
                foreach(var it in children)
                {
                    if(it.Inside(box))
                    {
                        return it.add(box);
                    }
                }
                objects.Add(box);
            }else if(ObjectCount() >= MAX_CHILD_COUNT &&
                depth < MAX_DEPTH
                )
            {
                // 需要扩展
                var center = (aabb.Max + aabb.Min) / 2.0f;

                // 这里假设上一步是已经宽松过的包围盒，将其除以 XNode.ENPAND_K
                var lenth = (aabb.Max - aabb.Min)/ (XNode.ENPAND_K * 4.0F);
                Point3d[] diff = new Point3d[8];
                diff[0] = new Point3d(lenth.X, lenth.Y, lenth.Z);
                diff[1] = new Point3d(-lenth.X, lenth.Y, lenth.Z);
                diff[2] = new Point3d(-lenth.X, -lenth.Y, lenth.Z);
                diff[3] = new Point3d(lenth.X, -lenth.Y, lenth.Z);
                diff[4] = new Point3d(lenth.X, lenth.Y, -lenth.Z);
                diff[5] = new Point3d(-lenth.X, lenth.Y, -lenth.Z);
                diff[6] = new Point3d(-lenth.X, -lenth.Y, -lenth.Z);
                diff[7] = new Point3d(lenth.X, -lenth.Y, -lenth.Z);

                var size = lenth * XNode.ENPAND_K ;

                for (int i = 0; i < 8; ++i)
                {
                    var xcen = center + diff[i];
                    children[i] = new XNode(
                        new Box(xcen+ size, xcen- size),
                        depth + 1);
                }
                
                add(box);

                // 尝试将所有box倾泻至孩子节点
                for(int i = objects.Count()-1; i >=0; --i)
                {
                    var obj = objects[i];
                    foreach (var ch in children)
                    {
                        if (ch.Inside(obj))
                        {
                            if(ch.add(obj))
                            {
                                objects.RemoveAt(i);
                                break;
                            }
                        }
                    }
                }
                
            }
            else
            {
                // 不能分裂
                objects.Add(box);
            }

            return true;
        }
    }


    public class XOCT
    {
       public  int AllObjectCount()
        {
            return root.AllObjectCount();
        }

        public XOCT()
        {
            root = new XNode(new Box(new Point3d(200, 200, 200),new Point3d(0, 0, 0)), 0);

            rand = new Random();
        }

        public bool add(Box box)
        {
            return root.add(box);
        }
        Random rand;

        XNode root;

        
        
        public Box NewBox()
        {
            int posX = rand.Next(60) + 60;
            int posY = rand.Next(60) + 60;
            int posZ = rand.Next(60) + 60;

            Point3d center = new Point3d(posX, posY, posZ);
            Point3d len = new Point3d();
            
            len.X = rand.Next(10) + 1;
            len.Y = rand.Next(10) + 1;
            len.Z = rand.Next(10) + 1;


            var ret = new Box(center + len/2.0F, center - len/2.0F);
            if (!root.Inside(ret))
            {
                throw new Exception("out of range");
            }
            return ret;
        }
    }
}
