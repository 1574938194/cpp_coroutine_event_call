from datetime import datetime  
import os  
import matplotlib.pyplot as plt  
import numpy as np  
import tensorflow as tf  
from six.moves import xrange  
data = np.load('final37.npy')  
data = data[:,:,0:60]  
#显示原始数据图像  
def Show_images(data,show_nums,save=False):  
    index = 0  
    for n in range(show_nums):  
        show_images = data[index:index+100]  
        show_images = show_images.reshape(100,3,60,1)  
        r,c = 10,10  
        fig,axs = plt.subplots(r,c)  
        cnt = 0  
        for i in range(r):  
            for j in range(c):  
                xy = show_images[cnt]  
                for k in range(len(xy)):  
                    x = xy[k][0:30]  
                    y = xy[k][30:60]  
                    if k == 0 :  
                        axs[i,j].plot(x,y,color='blue',linewidth=2)  
                    if k == 1:  
                        axs[i,j].plot(x,y,color='red',linewidth=2)  
                    if k == 2:  
                        axs[i,j].plot(x,y,color='green',linewidth=2)  
                        axs[i,j].axis('off')  
                cnt += 1  
        index += 100  
        if save:  
            if not os.path.exists('This_epoch'):  
                os.makedirs('This_epoch')  
            fig.savefig('This_epoch/%d.jpg' % n)  
            plt.close()  
        else:  
            plt.show()  
              
def Save_genImages(gen, epoch):  
    r,c = 10,10  
    fig,axs = plt.subplots(r,c)  
    cnt = 0  
    for i in range(r):  
        for j in range(c):  
            xy = gen[cnt]  
            for k in range(len(xy)):  
                x = xy[k][0:30]  
                y = xy[k][30:60]  
                if k == 0:  
                    axs[i,j].plot(x,y,color='blue')  
                if k == 1:  
                    axs[i,j].plot(x,y,color='red')  
                if k == 2:  
                    axs[i,j].plot(x,y,color='green')  
                    axs[i,j].axis('off')  
            cnt += 1  
    if not os.path.exists('gen_img1'):  
        os.makedirs('gen_img1')  
    fig.savefig('gen_img1/%d.jpg' % epoch)  
    plt.close()  
def Save_lossValue(epoch,iters,d_loss,g_loss):  
    with open('losst.txt','a') as f:  
        f.write("第%d个epoch,第%d个batch , d_loss: %.8f, g_loss: %.8f"%(epoch, iters, d_loss, g_loss)+'\n')  
def plot_loss(loss):  
    fig,ax = plt.subplots(figsize=(20,7))  
    losses = np.array(loss)  
    plt.plot(losses.T[0], label="Discriminator Loss")  
    plt.plot(losses.T[1], label="Generator Loss")  
    plt.title("Training Losses")  
    plt.legend()  
    plt.savefig('loss.jpg')  
    plt.show()  
#定义Relu激活函数  
def Relu(name, tensor):  
    return tf.nn.relu(tensor,name)  
  
#定义LeakyRelu激活函数  
def LeakyRelu(name, x, leak=0.2):  
    return tf.maximum(x,leak*x,name=name)  
  
#定义全连接层  
def Fully_connected(name, value, output_shape):  
    with tf.variable_scope(name,reuse=None) as scope:  
        shape = value.get_shape().as_list()  
        w = tf.get_variable('w',[shape[1],output_shape],dtype=tf.float32,  
                            initializer=tf.random_normal_initializer(stddev=0.01))  
        b = tf.get_variable('b',[output_shape],dtype=tf.float32,initializer=tf.constant_initializer(0.0))  
          
        return tf.matmul(value,w) + b  
      
#定义一维卷积  
def Conv1d(name, tensor, ksize, out_dim, stride, padding, stddev=0.01):  
    with tf.variable_scope(name):  
        w = tf.get_variable('w',[ksize,tensor.get_shape()[-1],out_dim],dtype=tf.float32,  
                            initializer=tf.random_normal_initializer(stddev=stddev))  
        var = tf.nn.conv1d(tensor,w,stride,padding=padding)  
        b = tf.get_variable('b',[out_dim],'float32',initializer=tf.constant_initializer(0.01))  
          
        return tf.nn.bias_add(var,b)  
      
#定义二维卷积  
def Conv2d(name, tensor, filter_size1 ,filter_size2, out_dim, stride1, stride2, padding, stddev=0.01):  
    with tf.variable_scope(name):  
        w = tf.get_variable('w',[filter_size1, filter_size2, tensor.get_shape()[-1], out_dim], dtype=tf.float32,  
                            initializer=tf.random_normal_initializer(stddev=stddev))  
        var = tf.nn.conv2d(tensor, w, [1, stride1, stride2, 1], padding=padding)  
        b = tf.get_variable('b',[out_dim], 'float32', initializer=tf.constant_initializer(0.01))  
          
        return tf.nn.bias_add(var,b)  
      
#定义二维反卷积  
def Deconv2d(name, tensor, filter_size1, filter_size2, outshape, stride1, stride2, padding, stddev=0.01):  
    with tf.variable_scope(name):  
        w = tf.get_variable('w', [filter_size1, filter_size2, outshape[-1], tensor.get_shape()[-1]], dtype=tf.float32,  
                                 initializer=tf.random_normal_initializer(stddev=stddev))  
        var = tf.nn.conv2d_transpose(tensor, w, outshape, strides=[1,stride1, stride2, 1], padding=padding)  
        b = tf.get_variable('b', [outshape[-1]],'float32', initializer=tf.constant_initializer(0.01))  
          
        return tf.nn.bias_add(var,b)  
def Get_inputs(real_size,noise_size):  
        real_img = tf.placeholder(tf.float32, [None, real_size], name='real_img')  
        noise_img = tf.placeholder(tf.float32, [None, noise_size], name='noise_img')  
          
        return real_img, noise_img  
      
def Generator(noise_img, reuse=False, alpha=0.01):  
    with tf.variable_scope('generator',reuse=reuse):  
#         print(noise_img.shape)  
        output = tf.layers.dense(noise_img,128)  
#         print(output.shape)  
        output = tf.maximum(alpha * output,output)  
        output = tf.layers.batch_normalization(output,momentum=0.8,training=True)  
        output = tf.layers.dropout(output, rate=0.25)  
          
        output = tf.layers.dense(output,512)  
        output = tf.maximum(alpha * output,output)  
        output = tf.layers.batch_normalization(output,momentum=0.8,training=True)  
        output = tf.layers.dropout(output,rate=0.25)  
          
        output = tf.layers.dense(output,180)  
        output = tf.tanh(output)  
        return output  
def Discriminator(img,reuse=False,alpha=0.01):  
      
    with tf.variable_scope("discriminator", reuse=reuse):  
        print(img.shape)  
        output = tf.layers.dense(img,512)  
        output = tf.maximum(alpha * output, output)  
          
        output = tf.layers.dense(output,128)  
        output = tf.maximum(alpha * output, output)  
          
        output = tf.layers.dense(output,1)  
        return output  
mode = 'gan' #gan, wgan, wgan-gp     
batch_size = 100  
epochs = 1  
n_sample = 100  
learning_rate = 0.0002  
lamda = 10  
img_size  = 180  
noise_size = 100  
  
tf.reset_default_graph()  
  
real_img, noise_img = Get_inputs(img_size,noise_size)#feed于此  
real_data = real_img  
fake_data = Generator(noise_img)  
  
disc_real = Discriminator(real_data,reuse=False)  
disc_fake = Discriminator(fake_data,reuse=True)  
  
  
#生成器和判别器中的tensor  
train_vars = tf.trainable_variables()  
g_vars = [var for var in train_vars if var.name.startswith("generator")]  
d_vars = [var for var in train_vars if var.name.startswith("discriminator")]  

#普通的GAN
if mode == 'gan':
    gen_cost = tf.reduce_mean(tf.nn.sigmoid_cross_entropy_with_logits(logits=disc_fake,labels=tf.ones_like(disc_fake))) #生成器loss
    disc_cost = tf.reduce_mean(tf.nn.sigmoid_cross_entropy_with_logits(logits=disc_fake,labels=tf.zeros_like(disc_fake)))
    disc_cost += tf.reduce_mean(tf.nn.sigmoid_cross_entropy_with_logits(logits=disc_real,labels=tf.ones_like(disc_real)))
    disc_cost /= 2. #判别器loss
    #优化器
    gen_train_op = tf.train.AdamOptimizer(learning_rate=2e-4, beta1=0.5).minimize(gen_cost,var_list=g_vars)
    disc_train_op = tf.train.AdamOptimizer(learning_rate=2e-4,beta1=0.5).minimize(disc_cost,var_list=d_vars)
    clip_disc_weights = None
    
#wgan
elif mode == 'wgan':
    gen_cost = -tf.reduce_mean(disc_fake) #生成器loss
    disc_cost = tf.reduce_mean(disc_fake) - tf.reduce_mean(disc_real) #判别器loss
    
    #优化器
    gen_train_op = tf.train.RMSPropOptimizer(learning_rate=5e-5).minimize(gen_cost,var_list=g_vars)
    disc_train_op = tf.train.RMSPropOptimizer(learning_rate=5e-5).minimize(disc_cost,var_list=d_vars)
    clip_ops = []
    #将判别器权重截断到[-0.01,0.01]
    for var in train_vars:
        if var.name.startswith("discriminator"):
            clip_bounds = [-0.01, 0.01]
            clip_ops.append(tf.assign(var,tf.clip_by_value(var,clip_bounds[0],clip_bounds[1])))
    clip_disc_weights = tf.group(*clip_ops)
    
elif mode == 'wgan-gp':
    gen_cost = -tf.reduce_mean(disc_fake) #生成器loss  
    disc_cost = tf.reduce_mean(disc_fake) - tf.reduce_mean(disc_real) #判别器loss 
       
    #梯度惩罚
    alpha = tf.random_uniform(shape=[batch_size,1],minval=0.,maxval=1.)  
    interpolates = alpha*fake_data + (1-alpha)*real_data  
    gradients = tf.gradients(Discriminator(interpolates,reuse=True),[interpolates])[0]  
    slopes = tf.sqrt(tf.reduce_sum(tf.square(gradients),reduction_indices=[1]))  
    gradient_penalty = tf.reduce_mean((slopes-1.)**2)  
    disc_cost += lamda * gradient_penalty  
    clip_disc_weights = None
  
    #优化器  
    gen_train_op = tf.train.AdamOptimizer(learning_rate=1e-4,beta1=0.5,beta2=0.9).minimize(gen_cost,var_list=g_vars)  
    disc_train_op = tf.train.AdamOptimizer(learning_rate=1e-4,beta1=0.5,beta2=0.9).minimize(disc_cost,var_list=d_vars)  
  
saver = tf.train.Saver()  
def Train():  
    losses = []  
    with tf.Session() as sess:  
        sess.run(tf.global_variables_initializer())  
        for e in range(epochs):  
            for i in xrange(len(data)//batch_size):  
                batch_images = data[i*batch_size:(i+1)*batch_size]  
                batch_images = batch_images.reshape(batch_size,180)  
                batch_images = batch_images*2 -1   
                batch_noise = np.random.uniform(-1,1,size=(batch_size,noise_size))
                if mode == 'gan': #普通的gan，判别器，生成器各训练一次
                    disc_iters = 2
                else:             #wgan和wgan-gp，判别器训练多次，生成器训练一次
                    disc_iters = 2  
                for x in range(0,disc_iters):  
                    _,d_loss = sess.run([disc_train_op,disc_cost],feed_dict={real_data:batch_images,noise_img:batch_noise}) 
                    if clip_disc_weights is not None:
                        _ = sess.run(clip_disc_weights) 
                _,g_loss = sess.run([gen_train_op,gen_cost],feed_dict={noise_img:batch_noise})  
                Save_lossValue(e,i,d_loss,g_loss)  
                print("第%d个epoch,第%d个batch , d_loss: %.8f, g_loss: %.8f"%(e, i, d_loss, g_loss))  
                losses.append((d_loss,g_loss))  
            sample_noise = np.random.uniform(-1,1,size=(100,100))  
            gen_samples = sess.run(Generator(noise_img,reuse=True),feed_dict={noise_img:sample_noise})  
            print(gen_samples.shape)  
            saver.save(sess,'checkpoints/test.ckpt')  
            if e % 1 == 0:  
                gen = gen_samples.reshape(100,3,60,1)  
                Save_genImages(gen, e)  
        plot_loss(losses)               
  
def Test():  
    saver = tf.train.Saver(var_list=g_vars)  
    with tf.Session() as sess:  
        saver.restore(sess,tf.train.latest_checkpoint("checkpoints"))  
#         saver.restore(sess,'checkppoints/b.ckpt')  
        sample_noise = np.random.uniform(-1, 1, size=(10000,noise_size))  
        gen_samples = sess.run(Generator(noise_img,reuse=True),feed_dict={noise_img:sample_noise})  
        gen_images = (gen_samples+1)/2  
        show_num = len(gen_images)//100  
        Show_images(gen_images,show_num,save=True)  
  
if __name__ == '__main__':  
    Train()  
    #Test()  
