package com.lordphilly.pstouch.nativeapp

import android.app.Activity
import android.content.ContentValues
import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.Path
import android.graphics.Typeface
import android.net.Uri
import android.os.Bundle
import android.os.Environment
import android.provider.MediaStore
import android.view.MotionEvent
import android.view.View
import android.widget.Toast
import kotlin.math.max

class MainActivity : Activity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(EditorView(this))
    }
}

/**
 * First native Plan B vertical slice.
 *
 * Android Canvas is backed by Skia. This prototype intentionally keeps the
 * editor engine small and self-contained while the document/layer model is
 * designed in the next milestone.
 */
private class EditorView(context: android.content.Context) : View(context) {
    private val density = resources.displayMetrics.density
    private val toolbarHeight = dp(72f)
    private val backgroundPaint = Paint(Paint.ANTI_ALIAS_FLAG)
    private val toolbarPaint = Paint(Paint.ANTI_ALIAS_FLAG)
    private val titlePaint = Paint(Paint.ANTI_ALIAS_FLAG)
    private val buttonPaint = Paint(Paint.ANTI_ALIAS_FLAG)
    private val buttonTextPaint = Paint(Paint.ANTI_ALIAS_FLAG)
    private val canvasPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.WHITE
        style = Paint.Style.FILL
    }
    private val strokePaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.rgb(25, 25, 28)
        style = Paint.Style.STROKE
        strokeCap = Paint.Cap.ROUND
        strokeJoin = Paint.Join.ROUND
        strokeWidth = dp(12f)
    }
    private val history = ArrayDeque<Bitmap>()
    private var document: Bitmap? = null
    private var documentCanvas: Canvas? = null
    private var activePath: Path? = null
    private var lastX = 0f
    private var lastY = 0f

    init {
        isFocusable = true
        toolbarPaint.color = Color.rgb(16, 17, 20)
        titlePaint.apply {
            color = Color.WHITE
            textSize = dp(21f)
            typeface = Typeface.create(Typeface.DEFAULT, Typeface.BOLD)
        }
        buttonPaint.color = Color.rgb(43, 46, 53)
        buttonTextPaint.apply {
            color = Color.rgb(232, 235, 242)
            textSize = dp(14f)
            typeface = Typeface.create(Typeface.DEFAULT, Typeface.BOLD)
        }
        backgroundPaint.color = Color.rgb(225, 228, 234)
        setBackgroundColor(backgroundPaint.color)
    }

    override fun onSizeChanged(width: Int, height: Int, oldWidth: Int, oldHeight: Int) {
        if (width <= 0 || height <= toolbarHeight) return
        val old = document
        val newBitmap = Bitmap.createBitmap(
            width,
            max(1, height - toolbarHeight.toInt()),
            Bitmap.Config.ARGB_8888,
        )
        val newCanvas = Canvas(newBitmap)
        newCanvas.drawColor(Color.WHITE)
        if (old != null && !old.isRecycled) {
            val source = android.graphics.Rect(0, 0, old.width, old.height)
            val target = android.graphics.Rect(0, 0, newBitmap.width, newBitmap.height)
            newCanvas.drawBitmap(old, source, target, canvasPaint)
            old.recycle()
        }
        document = newBitmap
        documentCanvas = newCanvas
        history.clear()
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        canvas.drawColor(backgroundPaint.color)
        canvas.drawRect(0f, 0f, width.toFloat(), toolbarHeight, toolbarPaint)

        canvas.drawText("PS Touch Native", dp(18f), dp(32f), titlePaint)
        canvas.drawText("Plano B • editor nativo", dp(18f), dp(55f), buttonTextPaint.apply {
            textSize = dp(11f)
            color = Color.rgb(170, 177, 190)
        })
        buttonTextPaint.textSize = dp(14f)
        buttonTextPaint.color = Color.rgb(232, 235, 242)

        val buttons = listOf("Novo", "Desfazer", "Limpar", "Exportar")
        val buttonWidth = dp(90f)
        val gap = dp(8f)
        var x = width - (buttonWidth + gap) * buttons.size + gap
        buttons.forEach { label ->
            val rect = android.graphics.RectF(x, dp(12f), x + buttonWidth, dp(60f))
            canvas.drawRoundRect(rect, dp(9f), dp(9f), buttonPaint)
            val textWidth = buttonTextPaint.measureText(label)
            val baseline = rect.centerY() - (buttonTextPaint.ascent() + buttonTextPaint.descent()) / 2f
            canvas.drawText(label, rect.centerX() - textWidth / 2f, baseline, buttonTextPaint)
            x += buttonWidth + gap
        }

        document?.let { bitmap ->
            canvas.drawBitmap(bitmap, 0f, toolbarHeight, canvasPaint)
        }
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        val x = event.x
        val y = event.y
        if (event.action == MotionEvent.ACTION_DOWN && y < toolbarHeight) {
            handleToolbarAction(x)
            return true
        }
        if (y < toolbarHeight || document == null) return true

        val documentY = y - toolbarHeight
        when (event.action) {
            MotionEvent.ACTION_DOWN -> {
                saveHistory()
                activePath = Path().apply { moveTo(x, documentY) }
                lastX = x
                lastY = documentY
                documentCanvas?.drawCircle(x, documentY, strokePaint.strokeWidth / 2f, canvasPaintForStroke())
                invalidate()
            }
            MotionEvent.ACTION_MOVE -> {
                val path = activePath ?: return true
                val midX = (lastX + x) / 2f
                val midY = (lastY + documentY) / 2f
                path.quadTo(lastX, lastY, midX, midY)
                documentCanvas?.drawPath(path, strokePaint)
                lastX = x
                lastY = documentY
                activePath = Path().apply { moveTo(lastX, lastY) }
                invalidate()
            }
            MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> {
                activePath = null
                invalidate()
            }
        }
        return true
    }

    private fun canvasPaintForStroke(): Paint = Paint(strokePaint).apply {
        style = Paint.Style.FILL
        color = strokePaint.color
    }

    private fun saveHistory() {
        document?.let { bitmap ->
            if (history.size >= 12) history.removeFirst().recycle()
            history.addLast(bitmap.copy(Bitmap.Config.ARGB_8888, false))
        }
    }

    private fun handleToolbarAction(x: Float) {
        val buttonWidth = dp(90f)
        val gap = dp(8f)
        val firstX = width - (buttonWidth + gap) * 4 + gap
        val index = ((x - firstX) / (buttonWidth + gap)).toInt()
        if (index !in 0..3) return
        when (index) {
            0 -> clearDocument(false)
            1 -> undo()
            2 -> clearDocument(true)
            3 -> exportPng()
        }
    }

    private fun clearDocument(saveUndo: Boolean) {
        if (saveUndo) saveHistory()
        documentCanvas?.drawColor(Color.WHITE)
        invalidate()
    }

    private fun undo() {
        val previous = history.removeLastOrNull() ?: return
        document?.let { current ->
            documentCanvas?.drawBitmap(previous, 0f, 0f, canvasPaint)
            previous.recycle()
            if (current.isRecycled) document = null
        }
        invalidate()
    }

    private fun exportPng() {
        val bitmap = document ?: return
        val name = "ps-touch-native-${System.currentTimeMillis()}.png"
        val values = ContentValues().apply {
            put(MediaStore.Images.Media.DISPLAY_NAME, name)
            put(MediaStore.Images.Media.MIME_TYPE, "image/png")
            put(
                MediaStore.Images.Media.RELATIVE_PATH,
                Environment.DIRECTORY_PICTURES + "/PS Touch Native",
            )
        }
        val resolver = context.contentResolver
        var uri: Uri? = null
        try {
            uri = resolver.insert(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, values)
            if (uri == null) error("MediaStore insert returned null")
            resolver.openOutputStream(uri)?.use { output ->
                if (!bitmap.compress(Bitmap.CompressFormat.PNG, 100, output)) {
                    error("PNG compression failed")
                }
            } ?: error("Could not open output stream")
            Toast.makeText(context, "PNG salvo em Pictures/PS Touch Native", Toast.LENGTH_LONG).show()
        } catch (error: Exception) {
            uri?.let { resolver.delete(it, null, null) }
            Toast.makeText(context, "Falha ao exportar: ${error.message}", Toast.LENGTH_LONG).show()
        }
    }

    private fun dp(value: Float): Float = value * density
}
