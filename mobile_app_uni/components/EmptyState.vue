<template>
	<view class="empty-state">
		<text class="empty-icon">{{ icon }}</text>
		<text class="empty-title">{{ title }}</text>
		<text class="empty-text">{{ text }}</text>
	</view>
</template>

<script>
export default {
	name: 'EmptyState',
	props: {
		icon: {
			type: String,
			default: '∅'
		},
		title: {
			type: String,
			default: '暂无数据'
		},
		text: {
			type: String,
			default: '当前没有可显示的内容'
		}
	}
}
</script>

<style>
.empty-state {
	padding: 48rpx 24rpx;
	display: flex;
	flex-direction: column;
	align-items: center;
	text-align: center;
}

.empty-icon {
	font-size: 56rpx;
	color: #94A3B8;
}

.empty-title {
	margin-top: 18rpx;
	font-size: 28rpx;
	font-weight: 800;
	color: #334155;
}

.empty-text {
	margin-top: 8rpx;
	font-size: 24rpx;
	color: #94A3B8;
}
</style>
