# Status of Supported OpenGL Command Overrides

## CORE IMMEDIATE MODE
- glBegin
- glEnd
- glVertex2f
- glVertex3f
- glColor3f
- glColor4f
- glNormal3f
- glTexCoord2f

## DISPLAY LISTS
- glCallList
- glNewList
- glEndList
- glGenLists

## CLIENT STATE
- **glEnableClientState**
- **glDisableClientState**
- **glVertexPointer**
- **glNormalPointer**
- **glTexCoordPointer**
- **glColorPointer**
- **glDrawArrays**
- **glDrawElements**

## MATRIX OPERATIONS
- glMatrixMode
- glLoadIdentity
- glLoadMatrixf
- glMultMatrixf
- glPushMatrix
- glPopMatrix
- glTranslatef
- glRotatef
- glScalef
- glViewport
- glOrtho
- glFrustum
- gluPerspective

## RENDERING
- glClear
- glClearColor
- glFlush
- glFinish
- glBindTexture
- glGenTextures
- glDeleteTextures
- glTexImage2D
- glTexParameterf
- **glTexEnvi**
- **glTexEnvf**

## FIXED FUNCTION
- glLightf
- glLightfv
- **glMateriali**
- glMaterialf
- **glMaterialiv**
- glMaterialfv
- **glAlphaFunc**

## STATE MANAGEMENT
- glEnable
- glDisable
- **glColorMask**
- **glDepthMask**
- **glBlendFunc**
- **glPointSize**
- **glPolygonOffset**
- **glCullFace**
- **glStencilMask**
- **glStencilFunc**
- **glStencilOp**
- **glStencilOpSeparateATI**

## WGL
- wglChoosePixelFormat
- wglDescribePixelFormat
- wglGetPixelFormat
- wglSetPixelFormat
- wglSwapBuffers
- wglCreateContext
- wglDeleteContext
- wglGetCurrentContext
- wglGetCurrentDC
- wglMakeCurrent
- wglShareLists
- wglSwapIntervalEXT